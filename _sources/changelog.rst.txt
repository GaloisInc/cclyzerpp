Changelog
=========

..
   See https://keepachangelog.com/en/1.0.0/ for a helpful reference.

`v0.7.0`_ - 2022-11-02
**********************

`Compare to v0.6.0 <https://github.com/GaloisInc/cclyzerpp/compare/v0.6.0...v0.7.0>`_.

Added
~~~~~

- Add limited support for LLVM 15 and nightly (16).
- Add support for ``fadd``, ``fsub``, ``fmin``, ``fmax`` atomic operations to
  the FactGenerator and Datalog.

Changed
~~~~~~~

- Changed look and feel of the documentation.
- Refactorings and small improvements to the FactGenerator.
- The FactGenerator fails more aggressively on malformed LLVM modules.

Removed
~~~~~~~

- Removed C pre-processor directives concerning LLVM versions less than 10. This
  may cause failures to build against very old versions of LLVM that were never
  officially supported.

`v0.6.0`_ - 2022-10-27
**********************

`Compare to v0.5.0 <https://github.com/GaloisInc/cclyzerpp/compare/v0.5.0...v0.6.0>`_.

Changed
~~~~~~~

- cclyzer++ now builds against LLVM 14. See :doc:`build` for how to build
  against other versions of LLVM.
- Considerable documentation improvements.
- The Docker images now use Ubuntu 22.04.
- Release artifacts are now built with optimizations.

Removed
~~~~~~~

- Removed handling of function attributes in Datalog and FactGenerator code.

`v0.5.0`_ - 2022-10-21
**********************

`Compare to v0.4.0 <https://github.com/GaloisInc/cclyzerpp/compare/v0.4.0...v0.5.0>`_.

Changed
~~~~~~~

- cclyzer++ now builds against (and requires) LLVM 11. It can still be built
  from source against LLVM 10, but the Debian package and Docker images all use
  LLVM 11 (see also `#98`_).

Fixed
~~~~~

- The Debian package now correctly lists its dependencies

`v0.4.0`_ - 2022-10-21
**********************

`Compare to v0.3 <https://github.com/GaloisInc/cclyzerpp/compare/v0.3...v0.4.0>`_.

Changed
~~~~~~~

- Improved type-safety in the Datalog code.
- Several relations were renamed for the sake of consistency between the
  FactGenerator and the analysis code (Datalog).

Removed
~~~~~~~

- Removed the unused ``subset-and-unification`` build target.
- Removed the instantiations of the ``ThrowInstruction`` component in both
  analyses.
- Removed a few extraneous relations.

`v0.3`_ - 2022-10-12
********************

`Compare to v0.2 <https://github.com/GaloisInc/cclyzerpp/compare/v0.2...v0.3>`_.

Added
~~~~~

- Added infrastructure for packaging for Debian with CMake and FPM.
- Added a new script that helps developers track down regressions, see
  ``scripts/run.py``.

Changed
~~~~~~~

- Many relations were renamed for the sake of consistency between the
  FactGenerator and the analysis code (Datalog).
- The repo was reorganized a bit, several files moved out of the top-level
  directory.
- Tests were reorganized and cleaned up.

Fixed
~~~~~

- Fixed bug that led to incorrect callgraphs in the presence of indirect calls
  (`#69`_).

`v0.2`_ - 2022-10-07
********************

`Compare to v0.1 <https://github.com/GaloisInc/cclyzerpp/compare/v0.1...v0.2>`_.

Added
~~~~~

- Aspirational developer documentation on naming conventions.
- Linting with Mypy in CI.

Changed
~~~~~~~

- Many relations were renamed for the sake of consistency between the
  FactGenerator and the analysis code (Datalog).
- Removed superfluous use of records in instruction schemata.

Removed
~~~~~~~

- FactGenerator no longer processes debug information.

Known Issues
~~~~~~~~~~~~

- May generate incorrect callgraphs in the presence of indirect calls, see
  `#69`_. Fixed in `v0.3`_.

`v0.1`_ - 2022-10-04
********************

First release!

Known Issues
~~~~~~~~~~~~

- May generate incorrect callgraphs in the presence of indirect calls, see
  `#69`_. Fixed in `v0.3`_.

.. _v0.1: https://github.com/GaloisInc/cclyzerpp/releases/tag/v0.1
.. _v0.2: https://github.com/GaloisInc/cclyzerpp/releases/tag/v0.2
.. _v0.3: https://github.com/GaloisInc/cclyzerpp/releases/tag/v0.3
.. _v0.4.0: https://github.com/GaloisInc/cclyzerpp/releases/tag/v0.4.0
.. _v0.5.0: https://github.com/GaloisInc/cclyzerpp/releases/tag/v0.5.0
.. _v0.6.0: https://github.com/GaloisInc/cclyzerpp/releases/tag/v0.6.0
.. _#69: https://github.com/GaloisInc/cclyzerpp/issues/69
.. _#98: https://github.com/GaloisInc/cclyzerpp/issues/98
