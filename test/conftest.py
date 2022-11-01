import gzip
import json
import os
import tempfile
from enum import Enum, unique
from itertools import chain, product
from os import getenv
from os.path import realpath
from pathlib import Path
from subprocess import check_call
from typing import Any, Dict, Final, List, Tuple
from uuid import uuid4

import pytest


@unique
class PointerAnalysis(str, Enum):
    DEBUG = "debug"
    SUBSET = "subset"
    UNIFICATION = "unification"


@unique
class ContextSensitivity(Enum):
    INSENSITIVE = "insensitive"
    CALLSITE1 = "1-callsite"
    CALLSITE2 = "2-callsite"
    CALLSITE3 = "3-callsite"
    CALLSITE4 = "4-callsite"
    CALLSITE5 = "5-callsite"
    CALLSITE6 = "6-callsite"
    CALLSITE7 = "7-callsite"
    CALLSITE8 = "8-callsite"
    CALLSITE9 = "9-callsite"
    CALLER1 = "1-caller"
    CALLER2 = "2-caller"
    CALLER3 = "3-caller"
    CALLER4 = "4-caller"
    CALLER5 = "5-caller"
    CALLER6 = "6-caller"
    CALLER7 = "7-caller"
    CALLER8 = "8-caller"
    CALLER9 = "9-caller"


PARENT: Path = Path(realpath(__file__)).parent
BUILD: Path = Path(getenv("CCLYZER_BUILD_DIR", str(PARENT.parent / "build")))
PROGRAMS_PATH: Path = PARENT.parent / "test" / "c"

# NOTE(ww): We don't use -Wall or -Werror here, since not all of our test
# programs compile successfully with them.
DEFAULT_COMPILER_FLAGS: Final[Tuple[str, ...]] = ("-O1",)

REQUIRED_CFLAGS: Final[Tuple[str, ...]] = ("-g", "-grecord-gcc-switches")


@pytest.fixture(scope="session")
def programs_path():
    return PROGRAMS_PATH


def _ir_for_program(
    program: Path, *, compiler_flags: Tuple[str, ...] = DEFAULT_COMPILER_FLAGS, **kwargs: Any
) -> Path:

    if program.suffix == ".cpp":
        compiler = "clang++"
    else:
        compiler = "clang"

    # The -fno-discard-value-names makes IR names more consistent across
    # compiles.
    combined_cflags = (
        *REQUIRED_CFLAGS,
        *compiler_flags,
        *kwargs.get("additional_cflags", tuple()),
        "-fno-discard-value-names",
    )
    mashed_flags = "".join(combined_cflags)

    ir_base = f"{Path(program.stem)}.{compiler}.{mashed_flags}.bc"
    ir_path = Path(tempfile.mkdtemp()) / ir_base
    check_call(
        [
            "clang",
            "-c",
            "-emit-llvm",
            *combined_cflags,
            program.name,
            "-o",
            ir_path,
        ],
        # HACK(ww): Run the compiler from the program directory to ensure
        # that the paths in our golden output are consistent.
        cwd=program.parent,
    )
    return ir_path


@pytest.fixture(autouse=True)
def run(programs_path):
    def _run(
        program: str,
        context_sensitivity: str = "1-callsite",
        signatures: Dict = dict(),
        **kwargs: Any,
    ) -> Path:
        ir_path = _ir_for_program(programs_path / program, **kwargs)
        out_path = ir_path.with_suffix(
            ".{}.{}.{}".format(context_sensitivity, str(uuid4())[:8], "pa-results")
        )

        signatures_path = ir_path.with_suffix(".{}.signatures.json".format(str(uuid4())))
        with open(signatures_path, mode="w") as f:
            json.dump(signatures, f)

        check_call(
            [
                "opt",
                "-load",
                BUILD / "libSoufflePA.so",
                "-load",
                BUILD / "libPAPass.so",
                "-disable-output",
                # TODO(lb): Use new pass manager
                "-enable-new-pm=0",
                "-cclyzer",
                "-signatures",
                signatures_path,
                "-debug-datalog=true",
                "-debug-datalog-dir={}".format(out_path),
                "-context-sensitivity={}".format(context_sensitivity),
                ir_path,
            ]
        )

        assert out_path.exists()
        assertion_relations = list(out_path.glob("assert_*.csv.gz"))
        assert len(assertion_relations) > 0
        for assert_relation in assertion_relations:
            assert gzip.decompress(assert_relation.read_bytes()) == b"", assert_relation

        os.sync()
        return out_path

    return _run


_GOLD_DIR: Path = PARENT / "gold"


@pytest.fixture(autouse=True)
def golden(programs_path, run):
    def _golden(
        program: str, relations: List[str], context_sensitivity: str = "1-callsite", **kwargs: Any
    ) -> Dict[str, Path]:
        ir_path = _ir_for_program(programs_path / program, **kwargs)
        out_path = None
        paths = dict()
        for relation in relations:
            # Golden outputs are stored uncompressed.
            relation_path = (_GOLD_DIR / ir_path.name).with_suffix(
                ".{}.{}.golden.csv".format(context_sensitivity, relation)
            )
            paths[relation] = relation_path

            # Create a new golden file if one doesn't already exist
            if not relation_path.exists():
                assert (
                    getenv("MAKE_GOLD_TESTS") is not None
                ), f"Missing golden test file: {relation_path}"
                if out_path is None:
                    out_path = run(program, context_sensitivity=context_sensitivity, **kwargs)
                assert out_path.exists()
                with open(relation_path, "w") as outfile:
                    with gzip.open(f"{out_path / relation}.csv.gz", "rt") as infile:
                        # Order doesn't matter
                        for line in sorted(infile.readlines()):
                            outfile.write(line)

        return paths

    return _golden


ProgramAndFlags = Tuple[str, Tuple[str, ...]]

CONTEXT_SENSITIVITY_FIXTURE_NAME: str = "context_sensitivity"

# HACK(ww): Don't run these programs in the `every_program` fixture.
# In general, the reason for this is that they don't compile with the default flags.
EVERY_PROGRAM_DENYLIST: Final[List[str]] = [
    "exception-driven-control-flow.cpp",
    "heap_oob.cpp",
]

CFLAGS: Final[List[Tuple[str]]] = [("-O0",)]  # TODO(881): Also test at -O1, -O2, and -O3
CPROGS: Final[List[str]] = [
    p.name for p in PROGRAMS_PATH.glob("*.c") if p.name not in EVERY_PROGRAM_DENYLIST
]
CXXPROGS: Final[List[str]] = [
    p.name for p in PROGRAMS_PATH.glob("*.cpp") if p.name not in EVERY_PROGRAM_DENYLIST
]
CPARAMS: Final[List[ProgramAndFlags]] = list(product(CPROGS, CFLAGS))
CXXPARAMS: Final[List[ProgramAndFlags]] = list(product(CXXPROGS, CFLAGS))

EVERY_PROGRAM: Final[List[ProgramAndFlags]] = list(chain(CPARAMS, CXXPARAMS))
SOME_PROGRAMS: Final[List[ProgramAndFlags]] = list(
    product(
        (
            "allocation-sizes.c",
            "cxxbasic.cpp",
            "functiontable.c",
            "notes.c",
            "points-to_context.c",
            "recurse.c",
            "virtual.cpp",
        ),
        CFLAGS,
    )
)

ONE_PROGRAM: Final[List[ProgramAndFlags]] = [SOME_PROGRAMS[0]]

EVERY_PROGRAM_FIXTURE_NAME: Final[str] = "every_program"
SOME_PROGRAMS_FIXTURE_NAME: Final[str] = "some_programs"


def pytest_generate_tests(metafunc):
    extra_tests = getenv("EXTRA_TESTS") == "1"
    if EVERY_PROGRAM_FIXTURE_NAME in metafunc.fixturenames:
        if extra_tests:
            metafunc.parametrize(EVERY_PROGRAM_FIXTURE_NAME, EVERY_PROGRAM)
        else:
            metafunc.parametrize(EVERY_PROGRAM_FIXTURE_NAME, ONE_PROGRAM)
    if SOME_PROGRAMS_FIXTURE_NAME in metafunc.fixturenames:
        if extra_tests:
            metafunc.parametrize(SOME_PROGRAMS_FIXTURE_NAME, SOME_PROGRAMS)
        else:
            metafunc.parametrize(SOME_PROGRAMS_FIXTURE_NAME, ONE_PROGRAM)
    if "optimization_flags" in metafunc.fixturenames:
        if extra_tests:
            metafunc.parametrize("optimization_flags", (("-O0",), ("-O1",), ("-O2",), ("-O3",)))
        else:
            metafunc.parametrize("optimization_flags", (("-O0",), ("-O2",)))

    if CONTEXT_SENSITIVITY_FIXTURE_NAME in metafunc.fixturenames:
        if getenv("EXTRA_TESTS") != "1":
            metafunc.parametrize(
                CONTEXT_SENSITIVITY_FIXTURE_NAME,
                (
                    ContextSensitivity.INSENSITIVE,
                    ContextSensitivity.CALLSITE1,
                    ContextSensitivity.CALLER1,
                ),
            )
        else:
            metafunc.parametrize(
                CONTEXT_SENSITIVITY_FIXTURE_NAME,
                (
                    ContextSensitivity.INSENSITIVE,
                    ContextSensitivity.CALLSITE1,
                    ContextSensitivity.CALLSITE2,
                    ContextSensitivity.CALLER1,
                    ContextSensitivity.CALLER2,
                ),
            )
