cclyzer++
=========

..
  The following text also appears in doc/index.rst. Changes here should be
  reflected there, and vice-versa.

cclyzer++ is a precise and scalable global pointer analysis for LLVM code. The
output of cclyzer++ can be used for a variety of program analysis tasks,
including:

* Creation of callgraphs with precise handling of indirect function calls and
  virtual method calls
* Precise inter-procedural control- and data-flow analysis
* Answering may-alias and must-not-alias queries

cclyzer++ is field- and array-sensitive, performs on-the-fly callgraph
construction, and supports many different configurations of context-sensitivity
including *k*-callsite sensitivity. It has subset-based (Andersen style) and
unification-based (Steensgaard style) analyses. cclyzer++ is written in `Soufflé
Datalog`_, and so is highly parallel. cclyzer++ was derived from `cclyzer`_.

See the `documentation <doc_>`_ for more information about cczlyer++, including
examples of its output. Documentation is also available `online`_.

If you use cclyzer++ in your own work, please include the following citations:

* Langston Barrett and Scott Moore. "cclyzer++: Scalable and Precise Pointer Analysis for LLVM." https://galois.com/blog/2022/08/cclyzer-scalable-and-precise-pointer-analysis-for-llvm/. 2022.
* George Balatsouras and Yannis Smaragdakis. "Structure-sensitive points-to analysis for C and C++." In Static Analysis: 23rd International Symposium, SAS 2016, Edinburgh, UK, September 8-10, 2016, Proceedings 23, pp. 84-104. Springer Berlin Heidelberg, 2016.

Acknowledgments
---------------

This material is based upon work supported by the United States Air Force and
Defense Advanced Research Project Agency (DARPA) under Contract No.
FA8750-19-C-0004. Any opinions, findings and conclusions or recommendations
expressed in this material are those of the author(s) and do not necessarily
reflect the views of the United States Air Force or DARPA. Approved for Public
Release, Distribution Unlimited.

.. _cclyzer: https://yanniss.github.io/cclyzer-sas16.pdf
.. _Soufflé Datalog: https://souffle-lang.github.io/
.. _doc: doc/index.rst
.. _online: https://galoisinc.github.io/cclyzerpp
