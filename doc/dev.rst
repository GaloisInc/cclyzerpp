Development
-----------

Running Tests
*************

The tests are run with `Pytest`_. They require compiling cclyzer++ first.

.. code-block:: bash

  cmake --build build -j $(nproc)
  pytest test/

To make new golden tests, set ``MAKE_GOLD_TESTS=1`` in the environment. To run
extra tests, set ``EXTRA_TESTS=1``.

Cutting a Release
*****************

First, ensure that the :doc:`changelog` is up to date. Then, push a new tag that
starts with ``v``, e.g., ``v0.2``, and the CI build will create a draft release
on Github. Worked example:

.. code-block:: shell

  git checkout main
  git pull
  git tag -a v0.1 -m v0.1
  git push --tags

After waiting for the CI build, manually edit the release as required and hit
"publish".

Naming Conventions
******************

Due to historical circumstance, cclyzer++ has many different naming conventions
in use in the Datalog code. The conventions in this section are prescriptive,
but only apply to new code.

Abbreviations
~~~~~~~~~~~~~

Names must strike a balance between brevity and comprehensibility. This section
enumerates abbreviations that are considered reasonable trade-offs in this
space. This judgment is highly subjective, but must be made somehow.

- ``asm``: Assembly
- ``block``: LLVM basic block
- ``const``: Constant
- ``global_var``: Global variable
- ``expr``: Expression
- ``func``: Function
- ``gep``: ``getelementptr``
- ``instr``: Instruction
- ``int``: Integer
- ``ptr``: Pointer
- ``var``: Variable
- ``vec``: Vector

Explicitly **not** further abbreviated:

- ``array``
- ``struct``
- ``type``

Performance Tuning
******************

Soufflé has `some <tuning>`_ `documentation <profiler>`_ on improving
performance. In addition to what's stated there, it's often helpful to look
at Soufflé's RAM IR by passing the ``--show=transformed-ram`` flag to Soufflé.
The RAM representation explicitly shows the effect of query plans (``.plan``
`directives <plan>`_ and `SIPS`_) and semi-naïve evaluation.

.. _tuning: https://souffle-lang.github.io/handtuning
.. _profiler: https://souffle-lang.github.io/profiler
.. _Pytest: https://docs.pytest.org
.. _SIPS: https://souffle-lang.github.io/handtuning#sideways-information-passing-strategy
