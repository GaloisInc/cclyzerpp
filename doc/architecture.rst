.. _architecture:

Architecture
============

cclyzer++ is written in Datalog. Like a SQL database, Datalog views the world in
terms of relations (AKA tables or sets of tuples). The inputs and outputs of
the analysis are directories of CSV files, each representing a relation.
cclyzer++ includes utilities to generate and consume these CSV files.

The Fact Generator
******************

The fact generator is an LLVM pass that consumes LLVM modules and outputs facts
that can be fed to the analysis. It doesn't do much pre-processing of the
information, rather just outputs a table-based view of the module's AST.

The fact generator is also used to configure the analysis parameters, namely the
type and amount of context sensitivity and points-to signatures. See the
:ref:`documentation on using the analysis <configuration>` for more
details.

.. _cpp:

C++ Interface
*************

The C++ interface is an LLVM pass that wraps both the fact generator and the
analysis itself. It can be used via ``opt`` (see :ref:`opt`) or as a C++
library. Its library interface exposes the core output relations (e.g.,
``var_points_to``) of the pointer analysis in terms of LLVM objects (e.g.,
``llvm::Value``) rather than their serialized string representations that appear
in the CSV files. See ``PointerAnalysis.h`` and ``PointerAnalysis.cpp`` for more
details on this interface.
