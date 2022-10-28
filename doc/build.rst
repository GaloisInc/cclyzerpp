Building
========

.. note::

  Most users don't need to build cclyzer++ from source: Pre-built Debian
  packages and Docker images are available. See :doc:`install` for more details.

Build Dependencies
******************

.. note::

  You can download a pre-built Docker image with all of the build dependencies
  rather than installing them on your host. See :doc:`docker` for details.

cclyzer++ requires Soufflé version 2.3; see the `Soufflé build documentation`_
for information on how to download and install Soufflé.

The build also requires a few Boost C++ libraries. Here's how to get them on
Ubuntu 20.04:

.. code-block:: bash

    apt-get install libboost-system-dev libboost-filesystem-dev libboost-iostreams-dev libboost-program-options-dev

Configuring the Build
*********************

There are several CMake options to configure the build:

- ``-DLLVM_MAJOR_VERSION=<MAJOR_VERSION_NUMBER>``: Build against a specific
  version of LLVM; see :ref:`LLVM Library Version <llvmlib>` for supported
  versions.

- ``-DUBSAN=1``: Build the FactGenerator with the undefined behavior sanitizer.

Building the Fact Generator
***************************

To build the fact generator, run

.. code-block:: bash

  cmake -G Ninja -B build -S .
  cmake --build build -j $(nproc) --target factgen-exe

You should now see an executable at ``build/factgen-exe``.

See :ref:`the architecture documentation <architecture>` for more information on
the role of the fact generator, and :ref:`the usage documentation
<configuration>` for details on running it.

Building the C++ Interface
**************************

To synthesize C++ code, compile it to a shared library, and compile the C++
interface, run

.. code-block:: bash

  cmake --build build -j $(nproc) --target PAPass

(You could also run CMake without specifying ``--target PAPass``.) The pass can
be run with ``opt``, see :ref:`the usage documentation <opt>`.

.. _Soufflé build documentation: https://souffle-lang.github.io/build
.. _releases page: https://github.com/GaloisInc/cclyzerpp/releases
