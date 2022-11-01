cclyzer++
=========

..
  The following text also appears in the top-level README.rst. Changes here
  should be reflected there, and vice-versa.

cclyzer++ is a precise and scalable global pointer analysis for LLVM code. The
output of cclyzer++ can be used for a variety of program analysis tasks,
including:

* Creation of callgraphs with precise handling of indirect function calls and
  virtual method calls
* Precise inter-procedural control- and data-flow analysis
* Answering may-alias and must-not-alias queries

See the :ref:`design documentation <design>` for further explanation and
examples of the output of cclyzer++.

cclyzer++ is field- and array-sensitive, performs on-the-fly callgraph
construction, and supports many different configurations of
:ref:`context-sensitivity <context-sensitivity>` including *k*-callsite
sensitivity. It has subset-based (Andersen style) and unification-based
(Steensgaard style) analyses. cclyzer++ is written in `Soufflé Datalog`_, and so
is highly parallel. cclyzer++ was derived from `cclyzer`_.

Documentation is also available `online`_.

cclyzer++ is actively developed and maintained by Galois, Inc.

.. note::
    You may also want to peruse the `Release Announcement
    <https://galois.com/blog/2022/08/cclyzer-scalable-and-precise-pointer-analysis-for-llvm/>`_
    and `Visual Guide to Pointer Analysis
    <https://galois.com/blog/2022/08/cclyzer/>`_ blog posts.

Table of Contents
-----------------

.. toctree::
   :maxdepth: 2
   :caption: User Documentation

   architecture
   changelog
   design
   docker
   implementation
   install
   overview
   signatures
   unsoundness
   usage

.. toctree::
   :maxdepth: 2
   :caption: Developer Documentation

   build
   dev
   legal

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

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
.. _online: https://galoisinc.github.io/cclyzerpp

.. _run-souffle: https://souffle-lang.github.io/execute
.. _plan: https://souffle-lang.github.io/rules#query-plan
