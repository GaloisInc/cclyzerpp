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

- Create branch with a name starting with ``release``.
- Ensure that the :doc:`changelog` is up to date.
- Bump the project version``CMakeLists.txt``.
- Check that CI was successful on the release branch.
- Merge the release branch to ``main``.
- Delete the release branch.
- Create and push a new tag on ``main`` that starts with ``v``, and ends with a
  semantic version i.e., ``vX.Y.Z``.
- Wait for CI, then publish the draft release it created.

Worked example:

.. code-block:: shell

  git checkout -b release
  # Make changes, push, wait, then merge release into main and delete release
  git checkout main
  git pull
  git tag -a v0.Y.Z -m v0.Y.Z
  git push --tags

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

Debugging Regressions
*********************

See ``scripts/run.py``.

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
