.. _unsoundness:

Sources of Unsoundness
======================

cclyzer++ is not completely sound. Instead, it prioritizes *usefulness* over
soundness, making it "soundy" (see http://soundiness.org/). The following
sections the discuss known, notable sources of unsoundness.

.. _suballoc-unsoundness:

Suballocations and Type Back-Propagation
****************************************

cclyzer++ creates :ref:`suballocations <suballocations>` based on an
allocation's type. However, :ref:`backprop` can result in an allocation having
multiple assigned types, and cclyzer++ doesn't consider aliasing between the
suballocations of the various "variants" of the allocation which have different
types. The following example [#f1]_ demonstrates this issue.

.. code-block:: c

  #include <stdint.h>
  #include <stdlib.h>

  typedef struct { int32_t i1; int32_t i2; } a;
  typedef struct { int16_t j1; int16_t j2; int32_t j3; } b;

  int main(int argc, char *argv[]) {
    a *p = malloc(sizeof(a));
    b *q = (b*)p;
    int32_t *x = &p->i1;
    int16_t *y = &q->j2;
    return 0;
  }

Due to :ref:`type back-propagation <backprop>`, cclyzer++ will create three
separate abstract allocations to represent allocations to represent the single
call to ``malloc``:

- An "untyped" allocation of type ``i8``, call it ``*heap_alloc@main[p]``
- An allocation of type ``a``, call it ``*typed_heap_alloc@main[p, a]``
- An allocation of type ``b``, call it ``*typed_heap_alloc@main[p, b]``

The allocation ``*heap_alloc@main[p, a]`` would have field suballocations
``*heap_alloc@main[p].i1`` and ``*heap_alloc@main[p].i2``, and similarly for
``*typed_heap_alloc@main[p, b]``. The issue is that the variable ``y`` points to
``*typed_heap_alloc@main[p, b].j2``, but the analysis doesn't keep track of the
overlap between ``*heap_alloc@main[p, b].j2`` and ``*heap_alloc@main[p, a].i1``,
so stores to one won't be reflected in loads from the other.

Language Coverage
*****************

cclyzer++ doesn't handle several tricky features of LLVM. The following features
could probably have an impact on the soundness of a pointer analysis results but
are not yet supported:

- Inline assembly
- The ``llvm.global_ctors`` and ``llvm.global_dtors`` intrinsic global variables
- Exception handling instructions:
    * ``catchpad``
    * ``catchret``
    * ``catchswitch``
    * ``cleanuppad``
    * ``cleanupret``
    * ``landingpad``
    * ``resume``
- Other difficult instructions:
    * ``addrspacecast``
    * ``callbr``
    * ``inttoptr``
    * ``ptrtoint``
    * ``va_arg``
- Several LLVM intrinsics:
    * ``llvm.memmove``
    * ``llvm.ptrmask``
    * ``llvm.stackrestore``
    * ``llvm.stacksave``
    * ``llvm.va_end``
    * ``llvm.va_start``
    * ``llvm.vp.ptrtoint``
- Code generator intrinsics
- Vector-related constant expressions, including
    * ``extractelement``
    * ``insertelement``
    * ``shufflevector``
- Vector-related instructions, including
    * ``extractelement``
    * ``insertelement``
    * ``shufflevector``
- Vector-related intrinsics, including
    * ``llvm.masked.load``
    * ``llvm.masked.store``
- Anything related to parallelism and concurrency, including instructions like
  ``cmpxchg``
- ``ptrtoint`` and ``inttoptr`` constant expressions
- The ``addrspacecast`` constant expression

The analysis also doesn't use any LLVM pointer metadata like ``llvm.lifetime.*``
or ``dereferencable`` to improve precision.

Poison and Undef
~~~~~~~~~~~~~~~~

The analysis doesn't do anything with poison or ``undef`` values - it doesn't
create points-to facts for them and it doesn't detect when they may be created
or how they may be propagated.

Type-Based Heuristics
*********************

cclyzer++ uses type-compatibility heuristics to filter out "infeasible" points-to
facts. In particular, the rules for handling ``memcpy`` and ``getelementptr``
use a notion of type compatibility to propagate fewer facts. This is probably
usually OK, but likely leads to unsoundness in similar situations to :ref:`the
one described for suballocations <suballoc-unsoundness>`.

External Functions
******************

External functions without sound :ref:`signatures <signatures>` may have
effects on points-to relations that aren't captured by the analysis.

.. rubric:: Footnotes

.. [#f1] Example adapted from: Sui, Y., Fan, X., Zhou, H. and Xue, J., 2018.
         Loop-oriented pointer analysis for automatic simd vectorization. *ACM
         Transactions on Embedded Computing Systems (TECS)*, 17(2), pp.1-31.
