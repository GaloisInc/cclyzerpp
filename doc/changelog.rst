Changelog
=========

..
   See https://keepachangelog.com/en/1.0.0/ for a helpful reference.

next
****

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
.. _#69: https://github.com/GaloisInc/cclyzerpp/issues/69
