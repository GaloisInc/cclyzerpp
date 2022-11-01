Overview
========

Notable Features
----------------

Subset and Unification Analyses
*******************************

There are two widely-used paradigms in constructing pointer analyses:
subset-based and unification-based. Unification-based analyses are generally
highly scalable, but less precise than comparable subset-based analyses.
cclyzer++ provides both.

See the :ref:`implementation documentation <impl-unification>` for information
about the unification-based analysis.

.. _conf-context-sensitivity:

Configurable Context Sensitivity and Heap Cloning
*************************************************

Context-sensitive pointer analyses are often more precise than
context-insensitive ones, but their performance is less predictable. cclyzer++
provides a variety of context sensitivity strategies and depths, including
*k*-callsite. The choice of context sensitivity can be :ref:`configured at
runtime <configuration>`.

cclyzer++ also performs :ref:`heap cloning <clone>`, matching whichever strategy
is chosen for context-sensitivity.

Modeling Library Functions
**************************

cclyzer++ supports modeling external functions with *signatures* which summarize
their effect on memory. Signatures can be used to model functions from libc or
other external libraries. Such models are crucial for soundness. See
:ref:`signatures` for details on how to create signatures.

Language Support
----------------

cclyzer++ has primarily been tested on LLVM code produced by Clang 10 through 14
when compiling from C and C++ for x86_64. Your mileage may vary with other
languages, compilers, and targets.

Bibliography
------------

The following papers significantly influenced the development of cclyzer++.

- Bravenboer, M. and Smaragdakis, Y., 2009, October. Strictly declarative
  specification of sophisticated points-to analyses. In *Proceedings of the 24th
  ACM SIGPLAN conference on Object oriented programming systems languages and
  applications* (pp. 243-262).
- Balatsouras, G. and Smaragdakis, Y., 2016, September. Structure-sensitive
  points-to analysis for C and C++. In *International Static Analysis Symposium*
  (pp. 84-104). Springer, Berlin, Heidelberg.

The pointer analysis `tutorial`_ of Smaragdakis and Balatsouras was also quite
helpful.

Comparison to cclyzer
---------------------

As mentioned above, cclyzer++ is based on cclyzer. The major differences are that
cclyzer++

* :ref:`supports LLVM 10 through 14 <llvmlib>`, with limited support for
  LLVM 15.
* is implemented in Soufflé rather than LogicBlox
* has :ref:`a C++ interface <cpp>`, rather than a Python one
* has runtime-configurable context-sensitivity and heap-cloning
* features a :ref:`unification-based analysis <impl-unification>`
* allows modeling external functions with :ref:`signatures <signatures>`
* drops certain uninformative contexts (see ``drop.dl``)

On the less impactful side, cclyzer++ also

* includes :ref:`assertions <assertions>` about correctness
* improves on the heuristic that controls array suballocations (see
  ``allocations-subobjects.dl``)
* tracks suballocation sizes and offsets (see ``allocations-subobjects.dl``)
* soundly handles ``argv``
* has additional documentation (both code-level comments and higher-level
  documents such as this one)

Many thanks to the authors of cclyzer for their contributions! The cclyzer
license is included with the source distribution of cclyzer++ as
``LICENSE-cclyzer.txt``.

Project Status
--------------

cclyzer++ is actively developed and maintained by Galois, Inc. Points of contact
are: Langston Barrett (email: langston at galois dot com) Scott Moore (email:
scott at galois dot com).

Versioning
----------

Since v0.4.0, cclyzer++ has attempted to follow `semantic versioning 2.0.0
<semver>`_.

.. _llvmlib:

LLVM Library Version
********************

.. TODO(lb): Policy for supporting different LLVM versions

cclyzer++ currently builds against LLVM 14 by default and can be built with
previous versions 13 through 10. cclyzer++ can be built with LLVM 15, but
the analysis `does not yet support opaque pointers <opaque_>`_.

Development Tools
*****************

cclyzer++ currently builds with Clang 12 or Clang 14 (including other Clang
tools such as clang-format and clang-tidy). There are `plans <llvmver_>`_ to
build with Clang 15.

.. _tutorial: http://yanniss.github.io/points-to-tutorial15.pdf
.. _llvmver: https://github.com/GaloisInc/cclyzerpp/issues/12
.. _opaque: https://github.com/GaloisInc/cclyzerpp/issues/113
.. _semver: https://semver.org/spec/v2.0.0.html
