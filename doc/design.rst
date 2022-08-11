.. _design:

Analysis Design
===============

Purpose
*******

The purpose of this document is twofold:

- To provide enough insight into pointer analysis and cclyzer++ specifically so
  that the reader can look at the output of the analysis and understand
  roughly what it means, allowing them to evaluate whether cclyzer++ is
  appropriate for their use-case.
- To detail the core design choices of cclyzer++ so that pointer analysis
  developers can read the code and understand how it works, and evaluate
  whether cclyzer++ is a suitable basis for an improved analysis.

The intended audience is someone who knows at least a bit about static
analysis and C programming, but may not be intimately familiar with pointer
analysis nor an expert in static analysis generally.

This discussion complements the in-code comments (rather than duplicating
them), since this document purposefully doesn't discuss implementation issues.

Introduction
************

The design space of pointer analyses is large and complex. Designs that achieve
an adequate trade-off between precision, soundness, and performance are the
subject of ongoing research. This section documents the choices made in
cclyzer++.

The following sections contain examples and some exposition of general pointer
analysis concepts, but are meant primarily to support the discussion of the
particulars of the design of cclyzer++. See the `tutorial`_ of Smaragdakis and
Balatsouras for a more thorough introduction to pointer analysis generally.

Examples are shown in C for readability, but cclyzer++ actually analyzes LLVM
code. Examples assume a subset-based analysis unless otherwise noted.

Abstraction
***********

In order to achieve termination and scalability, static analyses have to
*abstract*---that is, approximate---the usually infinite set of possible program
behaviors. Consider the following program fragment:

.. code-block:: c

  int main() {
    while (rand() % 2 == 0) {
      char *z = malloc(1);
      f(z);
    }
    return 0;
  }

The set of actual allocations made by this program at runtime is unbounded --
it's impossible to tell how many times the ``while`` loop and its call to
``malloc`` will be executed. Most pointer analyses, including cclyzer++,
approximate sets of allocations by their allocation site. In a C (or more
generally, LLVM) program, an allocation site can be:

1. A global variable
2. A stack allocation (e.g., a call to ``alloca`` or a local variable which has
   its address taken with ``&``)
3. A heap allocation (e.g., a call to ``malloc`` or ``realloc``)

In this document, we'll write these abstract allocations like so (using a scheme
that mimics how they appear in the output of cclyzer++):

- ``*global_alloc@global_var`` for a global variable ``global_var``.
- ``*stack_alloc@main[y]`` for a stack allocation in function ``main`` of a
  variable ``y`` [#f1]_
- ``*heap_alloc@main[z]`` for a heap allocation in function ``main`` assigned to
  a variable ``z``

The goal of a pointer analysis is then to compute three relations:

1. ``ptr_points_to``: A mapping from pointer-typed abstract allocations to
   abstract allocations they may point to
2. ``var_points_to``: A mapping from variables to the set of all abstract
   allocations they may point to
3. ``callgraph_edge``: The set of edges in the call graph

Example
~~~~~~~

Consider the following program:

.. code-block:: c

  char *global_var;

  int main() {
    while (1) {
      char y;
      char *z = malloc(1);
      global_var = z;
      f(global_var, &y, z);
    }
    return 0;
  }

A pointer analysis using an allocation-site abstraction would compute the
following results (also called *points-to facts*):

* ``ptr_points_to``:
    - ``*global_alloc@global_var`` points to ``*heap_alloc@main[z]``
* ``var_points_to``:
    - ``&global_var`` points to ``*global_alloc@global_var``
    - ``&y`` points to ``*stack_alloc@main[y]``
    - ``z`` points to ``*heap_alloc@main[z]``
* ``callgraph_edge``:
    - An edge from ``main`` to ``malloc`` at the callsite ``malloc(1)``
    - An edge from ``main`` to ``f`` at the callsite ``f(global_var, &y, z)``

An Aside on Assignments, Loads, and Stores
******************************************

The following discussion aims to be mostly agnostic to how analysis results are
computed. However, it will be helpful to look at the relationship between loads,
stores, ``ptr_points_to``, and ``var_points_to`` to understand the examples.

* After an assignment of pointer-typed variables like ``x = y``, ``x`` will
  ``var_points_to`` anything that ``y`` ``var_points_to``.
* After a store like ``*x = y``, ``x`` will ``ptr_points_to`` anything that
  ``y`` ``var_points_to``.
* After a load like ``x = *y``, then ``x var_points_to z`` if ``y var_points_to
  w`` and ``w ptr_points_to z``.

The examples below will illustrate how this works more concretely.

.. _suballocations:

Array- and Field-Sensitivity
****************************

The basic allocation site abstraction can lose precision when applied to
compound data structures like structs and arrays. cclyzer++ subdivides
allocations into *suballocations* to avoid imprecision.

This strategy is beneficial for precision, but can actually cause unsoundness
when combined with :ref:`backprop`. See :ref:`unsoundness` and `the cclyzer
paper <cclyzer>`_ for details.

Field Sensitivity
~~~~~~~~~~~~~~~~~

Consider the following program fragment:

.. code-block:: c

  int u;
  int v;
  struct { int *x; int *y; } w;
  w.x = &u;
  w.y = &v;
  int *z = w.x;

A *field-insensitive* analysis (one that uses the basic allocation site
abstraction) would conclude

* ``ptr_points_to``:
    - ``*stack_alloc@main[w]`` points to ``*stack_alloc@main[u]``
    - ``*stack_alloc@main[w]`` points to ``*stack_alloc@main[v]``
* ``var_points_to``:
    - ``&w`` points to ``*stack_alloc@main[w]``
    - ``z`` points to ``*stack_alloc@main[u]``
    - ``z`` points to ``*stack_alloc@main[v]``

The results for ``z`` are imprecise - at runtime, ``z`` can actually only point
to the stack allocation for ``u``.

In order to more precisely handle structs, cclyzer++ further divides struct
allocations into *field suballocations*, one for each field. For example, the
allocation ``*stack_alloc@main[w]`` would have suballocations
``*stack_alloc@main[w].x`` and ``*stack_alloc@main[w].y``. With field
suballocations, the pointer analysis results for this example are

* ``ptr_points_to``:
    - ``*stack_alloc@main[w].x`` points to ``*stack_alloc@main[u]``
    - ``*stack_alloc@main[w].y`` points to ``*stack_alloc@main[v]``
* ``var_points_to``:
    - ``&w`` points to ``*stack_alloc@main[w]`` [#f2]_
    - ``z`` points to ``*stack_alloc@main[u]``

Array Sensitivity
~~~~~~~~~~~~~~~~~

Consider the following program fragment:

.. code-block:: c

  char u;
  char v;
  char *arr[2];
  arr[0] = &u;
  arr[1] = &v;
  char *z = arr[0];

An *array-insensitive* analysis (one that uses the basic allocation site
abstraction) would conclude

* ``ptr_points_to``:
    - ``*stack_alloc@main[arr]`` points to ``*stack_alloc@main[u]``
    - ``*stack_alloc@main[arr]`` points to ``*stack_alloc@main[v]``
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]``
    - ``&v`` points to ``*stack_alloc@main[v]``
    - ``&arr`` points to ``*stack_alloc@main[arr]``
    - ``z`` points to ``*stack_alloc@main[u]``
    - ``z`` points to ``*stack_alloc@main[v]``

The results for ``z`` are imprecise - at runtime, ``z`` can only point to the
stack allocation for ``u``.

In order to precisely handle arrays and structs, cclyzer++ further divides
array-typed allocations into *array suballocations*. Arrays have some number of
suballocations at specific indices and a suballocation named ``[*]`` that
represents *any* index (equivalently, an unknown index), e.g., in the example
above ``*stack_alloc@main[arr]`` would have suballocations
``*stack_alloc@main[arr][0]``, ``*stack_alloc@main[arr][1]``, and
``*stack_alloc@main[arr][*]``. The results of an array-sensitive analysis for
this program would be

* ``ptr_points_to``:
    - ``*stack_alloc@main[arr][0]`` points to ``*stack_alloc@main[u]``
    - ``*stack_alloc@main[arr][1]`` points to ``*stack_alloc@main[v]``
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]``
    - ``&v`` points to ``*stack_alloc@main[v]``
    - ``&arr`` points to ``*stack_alloc@main[arr]``
    - ``arr[0]`` points to ``*stack_alloc@main[arr][0]``
    - ``z`` points to ``*stack_alloc@main[u]``

Loads from the ``[*]`` suballocation are treated as loading from all of the
indices at once, and the same is true (mutatis mutandis) for stores.

It's not the case that all possible array indices always have a suballocation -
this would be unscalable for even moderately sized arrays. Loads from and stores
to array indices that lack a dedicated suballocation fall back on the ``[*]``
suballocation. A heuristic decides which array indices should get a dedicated
suballocation, see ``allocations-subobjects.dl`` in the implementation for
details.

Array and field suballocations can be arbitrarily nested.

.. _context-sensitivity:

Context-Sensitivity
*******************

Consider the following program:

.. code-block:: c

  int *id(int *z) {
    return z;
  }

  int main() {
    int u;
    int v;
    int *x = id(&u);
    int *y = id(&v);
    return 0;
  }

A *context-insensitive* analysis (one that uses the basic allocation site
abstraction) would conclude

* ``ptr_points_to``: (empty)
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]``
    - ``&v`` points to ``*stack_alloc@main[v]``
    - ``z`` points to ``*stack_alloc@main[u]``
    - ``z`` points to ``*stack_alloc@main[v]``
    - ``x`` points to ``*stack_alloc@main[u]``
    - ``x`` points to ``*stack_alloc@main[v]``
    - ``y`` points to ``*stack_alloc@main[u]``
    - ``y`` points to ``*stack_alloc@main[v]``

The results for ``x`` and ``y`` are imprecise - at runtime, ``x`` can only point
to the stack allocation for ``u``, and ``y`` can only point to the stack
allocation for ``v``. The points-to results for ``u`` and ``v`` get combined
when they flow through the function ``id``.

To ameliorate this kind of imprecision, pointer analyses qualify points-to
results with *contexts*. The most common kind of context is a *k*-callsite
context: each points-to fact is qualified with a summary of the control flow
leading to the statement in question, namely, up to the last *k* callsites that
occurred before its execution. In this document, we'll write contexts like so:

- ``nil``: The empty context
- ``[main:11, nil]``: A context indicating that the last callsite was the 11th
  statement in the ``main`` function.
- ``[f:5, [g:8, nil]]``: A context indicating that the last callsite was the 5th
  statement of ``f``, and the call before that was a call to ``f`` from the 8th
  statement of ``g``.

Here's how a *k*-callsite sensitive analysis would analyze the above example for
any *k* > 0:

* ``ptr_points_to``: (empty)
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]`` in context ``nil``
    - ``&v`` points to ``*stack_alloc@main[v]`` in context ``nil``
    - ``z`` points to ``*stack_alloc@main[u]`` in context ``[main:3, nil]``
    - ``z`` points to ``*stack_alloc@main[v]`` in context ``[main:4, nil]``
    - ``x`` points to ``*stack_alloc@main[u]`` in context ``nil``
    - ``y`` points to ``*stack_alloc@main[v]`` in context ``nil``

When the context already has *k* entries and a call is encountered, the least
recent entry is dropped. For example, consider encountering a call at the third
statement of ``g`` in the context ``[f:2, nil]``. A 1-callsite-sensitive
analysis would switch to the context ``[g:3, nil]`` to analyze the callee,
whereas a 2-callsite-sensitive analysis would have the context ``[g:3, f:2,
nil]`` (and would drop ``f:2`` only at the *next* callsite it encountered). A
higher context depth usually leads to more precision, but may also make analysis
performance less predictable.

It can be advantageous to only *selectively* add callsites to the context. See
``drop.dl`` in the implementation for details.

.. _clone:

Heap Cloning
************

Consider the following program:

.. code-block:: c

  int **alloc() {
    int **w = malloc(sizeof(int *));
    return w;
  }

  int main() {
    int **x = alloc();
    int **y = alloc();
    int u;
    int v;
    *x = &u;
    *y = &v;
    int *z = *x;
    return 0;
  }

An analysis without heap cloning would conclude

* ``ptr_points_to``:
    - ``*heap_alloc@main[w]`` points to ``*stack_alloc@main[u]`` in context ``nil``
    - ``*heap_alloc@main[w]`` points to ``*stack_alloc@main[v]`` in context ``nil``
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]`` in context ``nil``
    - ``&v`` points to ``*stack_alloc@main[v]`` in context ``nil``
    - ``x`` points to ``*heap_alloc@main[w]`` in context ``nil``
    - ``y`` points to ``*heap_alloc@main[w]`` in context ``nil``
    - ``z`` points to ``*stack_alloc@main[u]`` in context ``nil``
    - ``z`` points to ``*stack_alloc@main[v]`` in context ``nil``

The results for ``z`` are imprecise - at runtime, ``z`` can only point to the
stack allocation for ``u``.

To ameliorate this kind of imprecision, pointer analyses qualify allocations
with *allocation contexts*, analogous to how points-to facts are qualified with
contexts. Instead of abstracting a set of allocations by an allocation site,
analyses with *heap cloning* abstract allocations by a *pair* of an allocation
site and an allocation context. Just as with context-sensitivity, the allocation
context is usually a summary of the control flow leading to the allocation site,
with *k*-callsite being a widely used choice. A 1-callsite sensitive analysis
with a 1-callsite-sensitive heap would analyze the above example as follows:

* ``ptr_points_to``:
    - (``*heap_alloc@main[w]``, ``[main:1, nil]``) points to
      ``*stack_alloc@main[u]`` in context ``nil``
    - (``*heap_alloc@main[w]``, ``[main:2, nil]``) points to
      ``*stack_alloc@main[v]`` in context ``nil``
* ``var_points_to``:
    - ``&u`` points to (``*stack_alloc@main[u]``, ``nil``) in context ``nil``
    - ``&v`` points to (``*stack_alloc@main[v]``, ``nil``) in context ``nil``
    - ``x`` points to (``*heap_alloc@main[w]``, ``[main:1, nil]``) in context
      ``nil``
    - ``y`` points to (``*heap_alloc@main[w]``, ``[main:2, nil]``) in context
      ``nil``
    - ``z`` points to (``*stack_alloc@main[u]``, nil) in context ``nil``

.. _backprop:

Type Back-Propagation
*********************

cclyzer++ infers types for heap allocations by looking at how they're used. For
instance, for the following code snippet

.. code-block:: c

  void *z = malloc(sizeof(int));
  // ...
  *((int*)z) = 4;

type back-propagation would infer that ``*heap_alloc@main[z]`` has type ``int``.
This type assignment can result in the creation of additional `suballocations
<suballocations_>`_ when cclyzer++ back-propagates an array or struct type. See
the `cclyzer`_ paper for more details.

Note that type back-propagation can result in multiple types being assigned to
the same allocation site, which can cause unsoundness in the analysis when
combined with :sub:`suballocations`. See :ref:`unsoundness` for details.

Undefined Behavior
******************

cclyzer++ assumes that the input program is free of undefined behavior, though in
practice the majority of the analysis results will still be useful in programs
containing such bugs. It's not at all clear how to soundly or precisely model
undefined behavior - by its very nature, it resists formalization in terms of
the semantics of the source language.

The assumption of fully defined behavior is rarely explicitly invoked in the
analysis, but it can help improve precision in certain cases. For example, when
the analysis sees a call to ``memcpy`` with a constant size argument, it ignores
any points-to facts involving source or target allocations that are smaller than
the size argument. If any of these allocations really did flow to the call to
``memcpy``, an out-of-bounds read or write would occur. Ignoring these ill-sized
facts improves precision, as new points-to facts are not generated for
impossible (i.e., undefined) data flows.

.. rubric:: Footnotes

.. [#f1] In all the examples here, the program is assumed to be in single static
         assignment (SSA) form, since this is the default for LLVM programs.
         Thus, the combination of function and variable name uniquely identify
         stack and heap allocation sites.
.. [#f2] Technically, ``&w`` would also point to ``*stack_alloc@main[u]`` since
         the first field of a struct aliases the overall struct allocation.

.. _cclyzer: https://yanniss.github.io/cclyzer-sas16.pdf
.. _tutorial: http://yanniss.github.io/points-to-tutorial15.pdf
