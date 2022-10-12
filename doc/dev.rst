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

Packaging cclyzer++
*******************

cclyzer++ is currently packaged for Debian derivatives using ``fpm``. a script
is available to automate this process, it takes the version number as an
argument:

.. code-block::

  ./pkg/pkg.sh 0.1

Cutting a Release
*****************

Before cutting a new release, consider running the extended test suite (see
``EXTRA_TESTS`` above).

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
- ``attr``: Attribute
- ``block``: LLVM basic block
- ``calling_conv``: Calling convention
- ``const``: Constant
- ``global_var``: Global variable
- ``expr``: Expression
- ``func``: Function
- ``gep``: ``getelementptr`` (this is the only abbreviated opcode)
- ``instr``: Instruction
- ``int``: Integer
- ``param``: Parameter
- ``ptr``: Pointer
- ``var``: Variable
- ``vec``: Vector

Explicitly **not** further abbreviated, because they are already fairly short,
or they are not common enough when speaking about LLVM to readily disambiguate
them from abbreviations for other terms:

- ``array``
- ``inl``: Could mean "inline" or "left injection"
- ``pred``: Could mean "predicate" or "predecessor"
- ``succ``: Could mean "successor" or "success"
- ``struct``

``type`` is for relations about types themselves, whereas ``ty`` is used for
relating an entity *to* its type.

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
