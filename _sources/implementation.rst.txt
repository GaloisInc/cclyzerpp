Analysis Implementation
=======================

cclyzer++ is written in `Soufflé Datalog`_ to achieve a balance between ease of
implementation and performance, see `Strictly declarative specification of
sophisticated points-to analyses
<https://dl.acm.org/doi/10.1145/1639949.1640108>`_ for the reasoning behind this
choice. cclyzer++ uses a variety of extensions to vanilla Datalog, each is
detailed below.

Context-Sensitivity via Records
*******************************

Contexts are represented as a cons-list of context items using Soufflé's
`records`_:

.. code-block::

  // From context/interface.dl
  .type ContextItem <: symbol
  .type Context = [
      head : ContextItem,
      tail : Context
  ]

In principle, there's no need to *k*-bound these contexts - one could instead
not insert repeated context entries and still guarantee termination in
principle. In practice, *k*-bounding is necessary for the analysis to terminate
in a reasonable time on all but the most trivial of programs.

The ``Context`` datatype represents both contexts and allocation contexts.

.. _impl-unification:

Unification-Based Analysis via Subsumption and Choice
*****************************************************

The unification-based analysis is fairly unusual. Such an analysis wouldn't
normally be written in vanilla Datalog, which lacks access to the datastructures
that make it performant (see discussion in the points-to `tutorial`_). However,
Soufflé provides features that make it possible to write a high-performance
unification-based analysis.

The unification-based analysis works by *merging* or *unifying* allocations that
flow to the same place. Consider the following program:

.. code-block:: c

  int main() {
    int u;
    int v;
    int *x;
    x = &u;
    x = &v;
    int *y = x;
    return 0;
  }

In the unification-based analysis, the allocations ``*stack_alloc@main[u]`` and
``*stack_alloc@main[v]`` both flow to the variable ``x``, so they would be
*unified*, and treated identically by the rest of the analysis. Ignoring
context-sensitivity and heap cloning for the sake of presentation, the output of
the analysis would conceptually be something like

* ``ptr_points_to``: (empty)
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]``
    - ``&v`` points to ``*stack_alloc@main[v]``
    - ``x`` points to *the set* ``{*stack_alloc@main[u], *stack_alloc@main[v]}``
    - ``y`` points to *the set* ``{*stack_alloc@main[u], *stack_alloc@main[v]}``

The stack allocations of ``u`` and ``v`` are said to be *merged*, *unified*, or
*in the same equivalence class*. This is represented by a binary relation
``unify`` over (allocation, allocation context) pairs (or just allocations when
not considering heap cloning).

Subsumption
~~~~~~~~~~~

Soufflé's ``eqrel`` `data structure <eqrel_>`_ seems suited to represent merged
allocations---a unification-based analysis featured in the evaluation section of
`the paper that introduced it <eqrel-paper_>`_. However, Galois `encountered
some performance issues <eqrel-perf_>`_ when implementing an analysis based on
``eqrel``. Luckily, Soufflé has a feature called `subsumption`_ which can be
used to emulate a union-find structure without some of the difficulties of
``eqrel``.

The following snippet schematically demonstrates how to build a union-find-like
structure using subsumption. ``base`` is some arbitrary binary relation over
numbers, which represents whatever pairs need to be inserted into an equivalence
relation. The relation ``union_find`` relates numbers (on the RHS) to their
equivalence class representatives (on the LHS). ``union_find`` eventually
guarantees that the equivalence class representative is the minimum of the
members of the equivalence class (though any way of uniquely choosing a
representative would do).

.. code-block::

  .decl base(x: number, y: number)
  .input base
  base(n, n) :- base(n, _); base(_, n).

  .decl eq(x: number, y: number) eqrel
  eq(x, y) :- base(x, y).

  .decl union_find(x: number, y: number) btree_delete
  union_find(x, y) :- base(x, y), x <= y.                   // rule #1
  union_find(y, x) :- base(x, y), y <= x.                   // rule #2
  union_find(x, z) :- union_find(x, y), union_find(y, z).   // rule #3
  union_find(x, y) <=                                       // rule #4
    union_find(z, y) :-
      z < x.

``union_find`` requires a bit more ceremony than ``eq``. Rules #1 and #2 insert
all the pairs in ``base``. Rule #3 states that if ``x`` is the representative of
the equivalence class of ``y`` and ``y`` is the representative of the
equivalence class of ``z``, then ``x`` is the representative of the equivalence
class of ``z``. This is analogous to what happens during the ``find`` operation
on union-find structures, when existing nodes' parents are modified. Rule #4
removes redundant facts using subsumption: if ``x`` and ``z`` are both potential
representative for ``y``'s equivalence class, and ``z`` is less than ``x``, then
delete the fact that says that ``x`` is a possible representative.

To check that ``eq`` and ``union_find`` really represent the same equivalence
relation, the following assertions may be added (they "pass" when they are
empty):

.. code-block::

  .decl assert_same1(x: number, y: number)
  assert_same1(x, y) :-
    eq(x, y),
    union_find(z, x),
    ! union_find(z, y).

  .decl assert_same2(x: number, y: number)
  assert_same2(x, y) :-
    union_find(z, x),
    union_find(z, y),
    ! eq(x, y).

Choice
~~~~~~

Besides a way to efficiently merge points-to sets, the other key component of a
performant unification-based analysis is a way to avoid propagating redundant
facts throughout the analysis (i.e., there should be only one points-to fact per
unified allocation set). cclyzer++ uses `Soufflé's ``choice-domain`` feature
<choice-domain_>`_ to non-deterministically choose a single (allocation,
allocation context) pair from each equivalence class for each (variable,
context) pair.

To illustrate this point, again consider the program from `the beginning of this
section <impl-unification_>`_ (and again ignore context sensitivity and heap
cloning). Then the unification analysis with ``choice-domain`` would
non-deterministically compute *either* of the following results:

* ``ptr_points_to``: (empty)
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]``
    - ``&v`` points to ``*stack_alloc@main[v]``
    - ``x`` points to ``*stack_alloc@main[u]``
    - ``y`` points to ``*stack_alloc@main[u]``
* ``unify``:
    - ``*stack_alloc@main[u]`` is unified with ``*stack_alloc@main[v]``

or

* ``ptr_points_to``: (empty)
* ``var_points_to``:
    - ``&u`` points to ``*stack_alloc@main[u]``
    - ``&v`` points to ``*stack_alloc@main[v]``
    - ``x`` points to ``*stack_alloc@main[v]``
    - ``y`` points to ``*stack_alloc@main[v]``
* ``unify``:
    - ``*stack_alloc@main[u]`` is unified with ``*stack_alloc@main[v]``

A post-processing step replaces each points-to fact with a *canonical* one,
where the canonical (allocation, allocation context) pair is the minimal one
under some arbitrary total order. If ``*stack_alloc@main[u]`` is less than
``*stack_alloc@main[v]`` under this order, then the "finalized" results
would have *all* the variables ``&u``, ``&v``, ``x``, and ``y`` pointing to
``*stack_alloc@main[u]``. See ``unification.dl`` on the details of total order
and finalization process.

Shared Code via Components
**************************

The subset- and unification-based analyses share code via `components`_. As a
representative example, the ``StripCtx`` component provides context-insensitive
projections of the core points-to relations. These rules are agnostic to the
style of the analysis, they only require that the "input relations"
(``callgraph_edge``, ``*_points_to``) have appropriate arities and types.

.. code-block::

  .comp StripCtx {
    .decl callgraph_edge(?calleeCtx: Context, ?callee: FunctionDecl, ?callerCtx: Context, ?callerInstr: Instruction) inline
    .decl operand_points_to(?aCtx: Context, ?alloc: Allocation, ?ctx: Context, ?operand: Operand) inline
    .decl ptr_points_to(?aCtx: Context, ?alloc: Allocation, ?ctx: Context, ?ptr: Allocation) inline
    .decl var_points_to(?aCtx: Context, ?alloc: Allocation, ?ctx: Context, ?var: Variable) inline

    .decl stripctx_callgraph_edge(?callee: FunctionDecl, ?callerInstr: Instruction)
    stripctx_callgraph_edge(?callee, ?callerInstr) :-
      callgraph_edge(_, ?callee, _, ?callerInstr).

    .decl stripctx_var_points_to(?alloc: Allocation, ?var: Variable)
    stripctx_var_points_to(?alloc, ?var) :-
      var_points_to(_, ?alloc, _, ?var).

    .decl stripctx_ptr_points_to(?to: Allocation, ?from: Allocation)
    stripctx_ptr_points_to(?to, ?from) :-
      ptr_points_to(_, ?to, _, ?from).

    .decl stripctx_operand_points_to(?to: Allocation, ?from: Operand)
    stripctx_operand_points_to(?to, ?from) :-
      operand_points_to(_, ?to, _, ?from).
  }

This component is then *instantiated* in the core pointer analysis component,
which both the subset- and unification-based analyses derive from. The points-to
relations are copied to the inputs of the ``StripCtx`` component.

.. code-block::

  .comp PointsTo {
    // ...

    .init stripctx = StripCtx

    stripctx.callgraph_edge(?calleeCtx, ?callee, ?callerCtx, ?callerInstr) :-
      callgraph.callgraph_edge(?calleeCtx, ?callee, ?callerCtx, ?callerInstr).

    stripctx.operand_points_to(?aCtx, ?alloc, ?ctx, ?operand) :-
      operand_points_to(?aCtx, ?alloc, ?ctx, ?operand).

    stripctx.ptr_points_to(?aCtx, ?alloc, ?ctx, ?ptr) :-
      ptr_points_to(?aCtx, ?alloc, ?ctx, ?ptr).

    stripctx.var_points_to(?aCtx, ?alloc, ?ctx, ?var) :-
      var_points_to(?aCtx, ?alloc, ?ctx, ?var).

    // ...
  }

  .comp SubsetPointsTo : PointsTo {
    // ...
  }

  .init subset = SubsetPointsTo

  .comp UnificationPointsTo : PointsTo {
    // ...
  }

  .init unification = UnificationPointsTo

  // Can refer to subset.stripctx.stripctx_callgraph_edge,
  // unification.stripctx.stripctx_var_points_to, etc.


In this way, both analysis have their own, separate version of these rules,
while sharing the implementation. Other components require different ways of
setting up their input relations depending on the analysis, these are
instantiated in ``subset.dl`` and ``unification.dl`` instead of in the
``PointsTo`` component.

All rules that depend (even indirectly) on the points-to relations are
parameterized on them. This means that the subset and unification analyses can
actually be run *at the same time* (or in sequence) during the same run of the
overall Datalog program. At the moment, this is mostly used for testing - in
particular, there are `assertions <assertions_>`_ to the effect that the results
of the unification analysis are a superset of those of the subset analysis.

.. _project:

Project Files
*************

Like C, the Soufflé language has no native notion of splitting a program into
multiple files. Soufflé uses the C pre-processor to concatenate many files into
a single translation unit. cclyzer++ has several top-level "project files" that
``#include`` different subsets of the Datalog files, for different purposes:

- ``debug.project`` includes all the files in cclyzer++, and exports almost all
  of the relations.
- ``subset.project`` includes only the files necessary to run the subset
  analysis and export the core points-to relations.
- ``unification.project`` includes only the files necessary to run the
  unification analysis and export the core points-to relations.

.. _assertions:

Assertions
**********

The file ``points-to/assertions.dl`` contains several "assertion relations",
which have names starting with ``assert_``. The assertions "fail" when they are
inhabited - but it's up to the consumer of the analysis to actually check if
this occurred and take appropriate action. Assertions are not included in the
points-to `project files <project_>`_, as they can be quite expensive to check
and are primarily intended for use in testing the analysis.

.. _Soufflé Datalog: https://souffle-lang.github.io/
.. _tutorial: http://yanniss.github.io/points-to-tutorial15.pdf
.. _records: https://souffle-lang.github.io/types#record-types
.. _eqrel: https://souffle-lang.github.io/relations#equivalence-relations
.. _eqrel-paper: https://ieeexplore.ieee.org/document/8891656
.. _eqrel-perf: https://github.com/souffle-lang/souffle/issues/2054
.. _subsumption: https://souffle-lang.github.io/subsumption
.. _choice-domain: https://souffle-lang.github.io/choice
.. _components: https://souffle-lang.github.io/components
