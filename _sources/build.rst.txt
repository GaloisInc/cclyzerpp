Building and Running
====================

cclyzer++ requires Soufflé version 2.3; see the `Soufflé build documentation`_
for information on how to download and install Soufflé.

The build also requires a few Boost C++ libraries. Here's how to get them on
Ubuntu 20.04:

.. code-block:: bash

    apt-get install libboost-system-dev libboost-filesystem-dev libboost-iostreams-dev libboost-program-options-dev

Building the Fact Generator
***************************

To build the fact generator, run

.. code-block:: bash

  cmake -G Ninja -B build -S .
  cmake --build build -j $(nproc) --target factgen-exe

You should now see an executable at ``build/factgen-exe``. See :ref:`the
architecture documentation <architecture>` for more information on the role of
the fact generator.

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

Run ``factgen-exe --help`` to see the full list of options.

Running the Analysis
********************

Soufflé ships with an interpreter and a *synthesizer*, which outputs C++ code
that can be compiled by Clang. Accordingly, there are three ways to run the
analysis:

* Directly via the Soufflé interpreter
* By synthesizing the Datalog to C++ and then either
  - compiling the C++ via Clang to a standalone executable
  - compiling the C++ via Clang to a shared library that gets loaded by LLVM's ``opt`` and used by the C++ interface

With Soufflé
~~~~~~~~~~~~

Here's how to run the subset analysis via the Soufflé interpreter:

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

.. _opt:

With Opt
~~~~~~~~

To synthesize C++ code, compile it to a shared library, and compile the C++
interface, install LLVM and run

.. code-block:: bash

  cmake --build build -j $(nproc) --target PAPass
  opt --disable-output --load=build/libSoufflePA.so --load=build/libPAPass.so -cclyzer --context-sensitivity=1-callsite --datalog-analysis=subset prog.bc

(You could also run CMake without specifying ``--target PAPass``.) Run
``opt --load=build/libSoufflePA.so --load=build/libPAPass.so --help`` to see
more options - though many of them are irrelevant.

.. _Soufflé build documentation: https://souffle-lang.github.io/build
