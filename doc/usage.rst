Usage
=====

See :doc:`install` for how to install cclyzer++.

.. _configuration:

Configuring the Analysis
************************

The analysis is configured via command-line arguments passed to the fact
generator, primarily ``--context-sensitivity`` and ``--signatures``. For
example,

.. code-block:: bash

  factgen-exe --out-dir <fact-dir> --signatures sigs.json --context-sensitivity 1-callsite prog.bc

where ``prog.bc`` is an LLVM bitcode module. The valid settings for context
sensitivity are:

* ``insensitive``
* ``1-callsite``
* ``2-callsite``
* ``3-callsite``
* ``4-callsite``
* ``5-callsite``
* ``6-callsite``
* ``7-callsite``
* ``8-callsite``
* ``9-callsite``
* ``1-caller``
* ``2-caller``
* ``3-caller``
* ``4-caller``
* ``5-caller``
* ``6-caller``
* ``7-caller``
* ``8-caller``
* ``9-caller``
* ``1-file``
* ``2-file``
* ``3-file``
* ``4-file``
* ``5-file``
* ``6-file``
* ``7-file``
* ``8-file``
* ``9-file``

Run ``factgen-exe --help`` to see the full list of options. See :ref:`the
architecture documentation <architecture>` for more information on the role of
the fact generator.

Running the Analysis
********************

Soufflé ships with an interpreter and a *synthesizer*, which outputs C++ code
that can be compiled by Clang. Accordingly, there are three ways to run the
analysis:

* Directly via the Soufflé interpreter
* By synthesizing the Datalog to C++ and then either

  - compiling the C++ via Clang to a standalone executable
  - compiling the C++ via Clang to a shared library that gets loaded by LLVM's ``opt`` and used by the C++ interface

The Debian package and ``dist`` Docker image only ship with the latter option.

.. _opt:

With Opt
~~~~~~~~

The LLVM pass is used via ``opt``. If you installed via the Debian package or
are running in the ``dist`` container, try:

.. code-block:: bash

  opt --disable-output --load=/usr/lib/libSoufflePA.so --load=/usr/lib/libPAPass.so -cclyzer --context-sensitivity=1-callsite --datalog-analysis=subset prog.bc

To see more options, run

.. code-block:: bash

  opt --load=/usr/lib/libSoufflePA.so --load=/usr/lib/libPAPass.so --help

though be warned - this includes all ``opt`` flags, and most of them are
irrelevant to running the analysis.

(If you built from source, the ``.so`` files will be in ``build/``.)

With Soufflé
~~~~~~~~~~~~

If you're not running in the ``dist`` Docker image and have the cclyzer++ source
available, you can run the analysis from the Soufflé interpreter. For example,
here's how to run the subset analysis:

.. code-block:: bash

  souffle --fact-dir <fact-dir> --output-dir <output-dir> datalog/subset.project

where ``fact-dir`` was the directory passed to the ``--out-dir`` option of the
fact generator. Pass ``-j <n>`` to parallelize the analysis across *n* threads.
See `the Soufflé documentation <run-souffle>`_ for more details.

To synthesize and compile the analysis, run

.. code-block:: bash

  souffle --generate=subset.cpp datalog/subset.project
  souffle-compile.py subset.cpp
  subset --facts <fact-dir> --output <output-dir> -j <n>
