#!/usr/bin/env python3
"""Find pointer analysis assertion failures and reduce the input programs.

There are a few manual steps to get this working. If working inside a Docker
container (recommended), ensure you've mounted /tmp to a tmpfs (RAM) for
performance:

  --mount type=tmpfs,dst=/tmp

Clone and build LLVM:

  git clone --depth 1 --branch main https://github.com/llvm/llvm-project
  pushd llvm-project
  cmake -S llvm -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLLVM_TARGETS_TO_BUILD=X86 \
    -DLLVM_ENABLE_ZLIB=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLLVM_ENABLE_LIBPFM=OFF \
    -DLLVM_PARALLEL_LINK_JOBS=4
  cmake --build build --target llvm-reduce
  cp build/bin/llvm-{as,dis,reduce} ..
  popd

Edit CMakeLists.txt to `include` the LLVM configuration from LLVM HEAD that you
just built instead of `find_package`ing the system version:

  include(/path/to/llvm-project/install/lib/cmake/llvm/LLVMConfig.cmake)
  # find_package(LLVM 10.0 REQUIRED CONFIG)

Build the FactGenerator with the new LLVM.

  cmake --build build -j $(nproc) --target factgen-exe

Get creduce too:

  apt-get update && apt-get install -qq -y creduce

Compile the pointer analysis to an executable. This takes a while, but the
extra optimizations are worth it here (-O3 and -march=native).

    souffle --generate=cclyzerpp.cpp datalog/debug.project && souffle-compile.py cclyzerpp.cpp

Now, configure this script.

The `--processes` flag controls how many different programs will be run through
Soufflé at once, they'll each have `--cores / --processes` Soufflé threads.
When you're just trying to find a program that causes an assertion to fail,
start with `--cores == --processes` unless/until you run out of RAM.

You may also want to adjust the Clang/Soufflé timeouts to your liking.

_DIRS controls which tests will be run, a good starting value is just
"sv-benchmarks/c". This script will clone the appropriate repo.

_INTERESTING_ASSERTIONS lists the files that you want to check are empty for
each run of Soufflé.

Note that this script caches compiled programs and the results of running
Soufflé on them in /tmp/reduce-py-cache. You can run

  ./reduce.py --reset

to blow the cache away. If you're modifying the pointer analysis, don't forget
to re-compile it, too.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from enum import Enum, unique
from functools import partial
from gzip import open as gzopen
from logging import getLogger, StreamHandler, DEBUG, INFO
from multiprocessing import Pool, cpu_count
from pathlib import Path
from shutil import copyfile, rmtree
from subprocess import check_call, check_output, STDOUT, CalledProcessError, TimeoutExpired
from os import environ, getcwd, walk
from resource import setrlimit, RLIMIT_AS, RLIM_INFINITY
from sys import argv
from tempfile import mkdtemp
from typing import List, Final, Optional, NewType, Tuple, Union
from time import time

logger = getLogger(__name__)

Bytes = NewType("Bytes", int)
Seconds = NewType("Seconds", int)
Minutes = NewType("Minutes", int)
Hours = NewType("Hours", int)

_DEFAULT_CORES: Final = cpu_count()
_MAX_VIRTUAL_MEMORY: Final = Bytes(int(500 * 1024 *
                                       1024))  # 500MB, per process


def minutes_to_seconds(mins: Minutes) -> Seconds:
    return Seconds(mins * 60)


_CLANG_TIMEOUT: Final[Seconds] = Seconds(10)
_SOUFFLE_TIMEOUT: Final[Seconds] = minutes_to_seconds(Minutes(30))


@unique
class Interesting(int, Enum):
    YES = 0
    NO = 1


@unique
class Result(str, Enum):
    INTERESTING = "interesting"
    UNINTERESTING = "uninteresting"
    UNCOMPILABLE = "uncompilable"


_C_SUFFIXES: Final = [".c"]
_CXX_SUFFIXES: Final = [".cpp", ".C"]  # GCC tests have .C files
_LLVM_SUFFIXES: Final = [".bc", ".ll"]


def is_c(prog: Path) -> bool:
    return prog.suffix in _C_SUFFIXES


def is_cxx(prog: Path) -> bool:
    return prog.suffix in _CXX_SUFFIXES


def is_llvm(prog: Path) -> bool:
    return prog.suffix in _LLVM_SUFFIXES


def is_source(prog: Path) -> bool:
    return is_c(prog) or is_cxx(prog)


def is_program(prog: Path) -> bool:
    return is_source(prog) or is_llvm(prog)


def looks_like_bitcode(path: Path) -> bool:
    """Returns whether the file at ``path`` looks like an LLVM bitcode module.

    This function is for quick and dirty checks; it does **not** validate that the file well-formed.
    """
    with path.open("rb") as io:
        magic = io.read(2)
    return magic == b"BC"


_THIS: Final = Path(__file__).resolve()
_PWD: Final = _THIS.parent
_DEFAULT_BUILD_ROOT: Final = _PWD / ".out/build"
_DEFAULT_ANALYSIS: Final = _PWD / "pointer-analysis"

_CACHE: Final = Path("/tmp/reduce-py-cache")
_CREDUCE: Final = Path("/tmp/reduce-py-cache/creduce")

_DIRS_TO_REPOS: Final = {
    "AnghaBench": "https://github.com/brenocfg/AnghaBench",
    "gcc": "https://github.com/gcc-mirror/gcc",
    "llvm-project": "https://github.com/llvm/llvm-project",
    "sv-benchmarks": "https://github.com/sosy-lab/sv-benchmarks",
}

_DIRS = list(
    map(
        lambda s: Path(_PWD / s),
        [
            # "sv-benchmarks/c",
            # "challenges-med",
            # "frontend/test/programs",
            # "llvm-project/clang/test/",  # Contains the below
            "llvm-project/clang/test/Sema",
            # "llvm-project/clang/test/SemaCXX",
            # "llvm-project/clang/test/CXX",
            # "gcc/gcc/testsuite/",  # Contains the below
            # "gcc-cxx-torture/",  # Weirdly, these files end in .C
            "gcc.c-torture/execute/",
            # "gcc.c-torture/compile/",

            # "AnghaBench/",
        ]))

for d in _DIRS:
    if not d.exists():
        for (parent, url) in _DIRS_TO_REPOS.items():
            if str(d).startswith(parent):
                cmd = ["git", "clone", "--depth=1", "--branch=master", url]
                logger.debug("Running %s", " ".join(str(s) for s in cmd))
                check_call(cmd)

    try:
        assert d.exists()
    except AssertionError:
        print("Does not exist:", d)
        raise

_INTERESTING_ASSERTIONS: Final[List[str]] = [
    # "assert_basic_or_path.csv.gz",
    # "assert_bitcast_operand_in_same_function.csv.gz",
    # "assert_ends_in_array_index_total.csv.gz",
    # "assert_every_allocation_has_a_region.csv.gz",
    # "assert_every_allocation_has_a_type.csv.gz",
    # "assert_every_allocation_has_one_region.csv.gz",
    # "assert_every_gep_constant_expr_points_to_something.csv.gz",
    # "assert_every_pointer_constant_points_to_something.csv.gz",
    # "assert_global_allocations_have_types",
    # "assert_has_repr.csv.gz",
    "assert_reachable_direct_calls_have_callees.csv.gz",
    "assert_subset_aliases_are_unification_aliases.csv.gz",
    "assert_subset_callgraph_edge_implies_unification_callgraph_edge.csv.gz",
    "assert_subset_reachable_ctx_implies_unification_reachable_ctx.csv.gz",
    "assert_subset_var_points_to_inhabited_implies_unification.csv.gz",
    # "assert_type_compatible_constant_points_to.csv.gz",
    # "assert_type_compatible_ptr_points_to.csv.gz",
    # "assert_type_compatible_var_points_to.csv.gz",
    "assert_unification_var_points_to_unique.csv.gz",
    "assert_unique_repr.csv.gz",
    "assert_var_points_to_implies_reachable.csv.gz",
    # "assert_gep.csv.gz",
    # "assert_subset_callgraph_edge_implies_unification_callgraph_edge.csv.gz",
    # "assert_subset_aliases_are_unification_aliases.csv.gz",
    # "assert_subset_var_points_to_inhabited_implies_unification.csv.gz"
    # "assert_pack_unify1",
    # "assert_pack_unify2"
]

_NUM_INTERESTING_ASSERTIONS: Final[int] = len(_INTERESTING_ASSERTIONS)


def limit_virtual_memory(
        max_virtual_memory: Bytes = _MAX_VIRTUAL_MEMORY):  # 1GB
    # The tuple below is of the form (soft limit, hard limit). Limit only
    # the soft part so that the limit can be increased later (setting also
    # the hard limit would prevent that).
    # When the limit cannot be changed, setrlimit() raises ValueError.
    setrlimit(RLIMIT_AS, (max_virtual_memory, RLIM_INFINITY))


def disassemble(path: Path) -> Path:
    assert is_llvm(path), f"Expected .bc or .ll file at {path}"
    if not path.suffix == ".ll":
        assert path.suffix == ".bc"
        cmd = [_PWD / "llvm-dis", path]
        logger.debug("Running %s", " ".join(str(s) for s in cmd))
        check_output(cmd,
                     timeout=_CLANG_TIMEOUT,
                     preexec_fn=limit_virtual_memory)
        path = path.with_suffix(".ll")
    assert path.exists(), f"Expected a file at {path} after running llvm-dis"
    return path


def assemble(path: Path) -> Path:
    assert is_llvm(path), f"Expected .bc or .ll file at {path}"
    if not path.suffix == ".bc":
        assert path.suffix == ".ll"
        cmd = [_PWD / "llvm-as", path]
        logger.debug("Running %s", " ".join(str(s) for s in cmd))
        check_output(cmd,
                     timeout=_CLANG_TIMEOUT,
                     preexec_fn=limit_virtual_memory)
        path = path.with_suffix(".bc")
    assert path.exists(), f"Expected a file at {path} after running llvm-as"
    assert looks_like_bitcode(path), f"Expected LLVM bitcode at {path}"
    return path


def run_fact_generator(config: Config, facts_dir: Path, path: Path) -> None:
    assert is_llvm(path)
    assert facts_dir.is_dir()

    cmd: List[Union[str, Path]] = [
        config.build_root / "llvm/PointerAnalysis/PointerAnalysis/factgen-exe",
        "--out-dir", facts_dir, "--context-sensitivity", "1-callsite", path
    ]
    logger.debug("Running %s", " ".join(str(s) for s in cmd))
    check_output(cmd,
                 stderr=STDOUT,
                 timeout=_SOUFFLE_TIMEOUT,
                 preexec_fn=limit_virtual_memory)

    assert is_llvm(path)
    assert facts_dir.is_dir()


def run_interpreted_analysis(config: Config, facts_dir: Path,
                             results_dir: Path) -> None:
    assert facts_dir.is_dir()
    assert results_dir.is_dir()

    cmd = [
        "souffle", "-j",
        str(config.cores // config.processes), "-L",
        config.build_root / "llvm", "-lFunctors", "-PSIPS:max-bound",
        _PWD / "llvm/PointerAnalysis/datalog/debug.project", "-F", facts_dir,
        "-D", results_dir
    ]
    logger.debug("Running %s", " ".join(str(s) for s in cmd))
    check_output(cmd,
                 cwd=_PWD,
                 stderr=STDOUT,
                 timeout=_SOUFFLE_TIMEOUT,
                 preexec_fn=limit_virtual_memory)

    assert facts_dir.is_dir()
    assert results_dir.is_dir()


def run_compiled_analysis(config: Config, facts_dir: Path,
                          results_dir: Path) -> None:
    assert facts_dir.is_dir()
    assert results_dir.is_dir()

    cmd = [
        config.analysis, "-j",
        str(config.cores // config.processes), "-F", facts_dir, "-D",
        results_dir
    ]
    logger.debug("Running %s", " ".join(str(s) for s in cmd))
    check_output(cmd,
                 stderr=STDOUT,
                 timeout=_SOUFFLE_TIMEOUT,
                 preexec_fn=limit_virtual_memory)

    assert facts_dir.is_dir()
    assert results_dir.is_dir()


def is_interesting(config: Config, path: Path) -> Interesting:
    assert is_llvm(path)
    path = assemble(path)
    assert path.suffix == ".bc"

    start = time()
    facts_dir = Path(mkdtemp(prefix=path.name, suffix=".facts"))
    results_dir = Path(mkdtemp(prefix=path.name, suffix=".results"))

    try:
        run_fact_generator(config, facts_dir, path)
    except CalledProcessError as e:
        logger.warning(e.output.decode("utf-8", errors="ignore"))
        logger.warning("Fact generator crash!")
        path.with_suffix(".souffle-crash").touch()
        if not config.debug:
            rmtree(facts_dir)
        return Interesting.NO
    except TimeoutExpired:
        logger.warning("Fact generator timeout!")
        if not config.debug:
            rmtree(facts_dir)
        return Interesting.NO

    try:
        if config.interpreter:
            run_interpreted_analysis(config, facts_dir, results_dir)
        else:
            # See the docstring
            run_compiled_analysis(config, facts_dir, results_dir)
    except CalledProcessError as e:
        logger.warning(e.output.decode("utf-8", errors="ignore"))
        logger.warning("Soufflé crash!!")
        path.with_suffix(".souffle-crash").touch()
        if not config.debug:
            rmtree(facts_dir)
            rmtree(results_dir)
        return Interesting.YES
    except TimeoutExpired:
        logger.warning("Soufflé timeout!")
        if not config.debug:
            rmtree(facts_dir)
            rmtree(results_dir)
        return Interesting.NO

    for assertion in _INTERESTING_ASSERTIONS:
        assertion_path = results_dir / assertion
        assert assertion_path.exists(), f"Missing assertion: {assertion}"
        with gzopen(assertion_path, mode="r") as assertion_data:
            assertion_content = assertion_data.read()
        if assertion_content != b"":
            if not config.debug:
                rmtree(facts_dir)
                rmtree(results_dir)
            logger.debug(f"Interesting! Assertion failed: {assertion_path}")
            return Interesting.YES
    # print(f"Done, {int(time() - start)}s elapsed")
    if not config.debug:
        rmtree(facts_dir)
        rmtree(results_dir)
    logger.debug("Not interesting.")
    return Interesting.NO


def compile(prog: Path, cflags=["-O0"]) -> Tuple[Path, bool]:
    if not is_c(prog) and not is_cxx(prog):
        assert is_llvm(prog)
        return (prog, True)

    clang = "clang"
    if is_cxx(prog):
        clang = "clang++"

    out = _CACHE / Path(prog.with_suffix("." + ".".join(cflags) + ".bc").name)
    cmd = [
        clang, "-fno-rtti", "-fno-discard-value-names", "-emit-llvm", "-o",
        out, "-c", prog
    ] + cflags
    if "sv-benchmarks" in str(prog.parent):
        cmd += [
            "-I", _PWD /
            "sv-benchmarks/c/Juliet_Test/Juliet_Test_Suite_v1.3_for_C_Cpp/C/testcasesupport"
        ]
    if not out.exists():
        logger.debug("Running %s", " ".join(str(s) for s in cmd))
        try:
            check_output(cmd,
                         stderr=STDOUT,
                         timeout=_CLANG_TIMEOUT,
                         preexec_fn=limit_virtual_memory)
        except TimeoutExpired:
            return (out, False)
        except CalledProcessError:
            return (out, False)

    assert out.exists(), f"Command: {' '.join(str(s) for s in cmd)}"
    assert looks_like_bitcode(out)
    return (out, True)


def compile_variants(prog: Path) -> List[Tuple[Path, bool, List[str]]]:
    if not is_source(prog):
        assert is_llvm(prog)
        return [(prog, True, [])]
    return [
        compile(prog, cflags=cflags) + (cflags, )
        for cflags in [["-O0"], ["-O2"]]
    ]


def get_cached_result_path(prog: Path, result: Result) -> Path:
    if result is Result.UNINTERESTING:
        return _CACHE / prog.with_suffix(".uninteresting").name
    elif result is Result.UNCOMPILABLE:
        return _CACHE / prog.with_suffix(".nocompile").name
    else:
        assert False


def rm_result(prog: Path, result: Result) -> None:
    path = get_cached_result_path(prog, result)
    if path.exists():
        path.unlink()


_CHECKED: Final[str] = f"(checked {_NUM_INTERESTING_ASSERTIONS} assertions)"


def show_result(prog: Path, result: Result) -> None:
    if result is Result.INTERESTING:
        logger.info(prog.name + ": interesting! ---------")
    elif result is Result.UNINTERESTING:
        logger.info(prog.name + f": uninteresting. {_CHECKED}")
    elif result is Result.UNCOMPILABLE:
        logger.info(prog.name + ": uncompilable.")
    else:
        assert False, f"Result was: {result} of type {type(result)}"


def save_result(prog: Path, result: Result) -> None:
    get_cached_result_path(prog, result).touch()


def get_cached_result(prog: Path) -> Optional[Result]:
    assert prog.exists()
    if get_cached_result_path(prog, Result.UNINTERESTING).exists():
        return Result.UNINTERESTING
    if get_cached_result_path(prog, Result.UNCOMPILABLE).exists():
        return Result.UNCOMPILABLE
    return None


def llvm_reduce(config: Config, compiled: Path) -> None:
    assert is_llvm(compiled)
    reduced = Path('reduced-' + compiled.with_suffix(".ll").name)
    if reduced.exists():
        return Result.INTERESTING
    cmd: List[Union[str, Path]] = [
        _PWD / "llvm-reduce", f"--test=./{__file__}",
        "--test-arg=--no-reduce",
        compiled
    ] + (["--test-arg=--interpreter"]  if config.interpreter else [])
    logger.debug("Running %s", " ".join(str(s) for s in cmd))
    check_call(cmd, preexec_fn=limit_virtual_memory)
    copyfile("reduced.ll", _PWD / reduced)
    assemble(_PWD / reduced)


def c_reduce(config: Config, source: Path, flags: List[str]) -> None:
    assert is_source(source)
    (compiled, success) = compile(source, cflags=flags)
    assert success, f"Couldn't compile source file: {source}"
    reduced = Path('creduced-' + source.name)
    if reduced.exists():
        return Result.INTERESTING
    cmd: List[Union[str, Path]] = ["creduce", __file__, source]
    logger.debug("Running %s", " ".join(str(s) for s in cmd))

    # creduce doesn't actually pass *any* arguments to the interestingness
    # script, so the only way to distinguish that we're being called by creduce
    # is to use a file (or environment variable, but that doesn't seem to work.)
    _CREDUCE.touch()

    check_call(cmd, preexec_fn=limit_virtual_memory)
    # TODO: Does creduce modify in-place?
    # copyfile("reduced.ll", _PWD / reduced)


def check(config: Config,
          compiled: Path,
          flags: List[str],
          source: Optional[Path] = None) -> Result:
    assert is_llvm(compiled)
    assert source is None or is_source(source)

    cached = get_cached_result(compiled)
    if cached is not None:
        return cached

    if is_interesting(config, compiled) is Interesting.YES:
        interesting = _PWD / ("interesting-" + compiled.name)
        copyfile(compiled, interesting)
        # TODO: Doesn't quite work yet...
        # if source is not None:
        #     c_reduce(config, source, flags)
        # else:
        llvm_reduce(config, compiled)
        return Result.INTERESTING

    result = Result.UNINTERESTING
    save_result(compiled, result)
    if not config.debug:
        compiled.unlink()
    return result


def check_variants(config: Config, prog: Path) -> List[Tuple[Path, Result]]:
    assert is_program(prog)

    source = None
    if is_source(prog):
        source = prog

    out = []
    for (variant, works, flags) in compile_variants(prog):
        if works is False:
            save_result(prog, Result.UNCOMPILABLE)
            out.append((variant, Result.UNCOMPILABLE))
            continue

        out.append((variant, check(config, variant, flags, source=source)))

    assert len(out) > 0
    return out


def check_and_show(config: Config, prog: Path) -> None:
    assert is_program(prog)
    logger.debug("Checking program at %s", prog)
    try:
        for (path, result) in check_variants(config, prog):
            show_result(path, result)
    except KeyboardInterrupt:
        pass


def find_programs(dirs=_DIRS,
                  suffixes=_C_SUFFIXES + _CXX_SUFFIXES + _LLVM_SUFFIXES):
    for d in dirs:
        for root, dirnames, filenames in walk(d):
            for filename in filenames:
                for suffix in suffixes:
                    if filename.endswith(suffix):
                        yield Path(root) / filename


def go(config: Config, path_str: str) -> Interesting:
    # This was called manually, not from inside this script, so we can reduce.
    path = Path(path_str)
    if not config.no_reduce:
        for (path, result) in check_variants(config, path):
            show_result(path, result)
        return Interesting.YES

    path = Path(path_str)
    paths = []
    if is_source(path):
        for (path, worked, _) in compile_variants(path):
            assert worked, f"Couldn't compile program at {path}"
            paths.append(path)
    else:
        paths.append(path)
    for path in paths:
        prog = disassemble(path)
        logger.info("Checking program at %s", prog)
        assert prog.exists()
        assert prog.suffix == ".ll"
        is_int = is_interesting(config, prog)
        print(is_int)
        if is_int == Interesting.YES:
            return is_int
    return Interesting.NO


def argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    parser.add_argument("-d",
                        "--debug",
                        action="store_true",
                        help="print verbose logging information")
    parser.add_argument(
        "-c",
        "--cores",
        type=int,
        default=_DEFAULT_CORES,
        help="number of cores to use",
    )
    parser.add_argument(
        "-p",
        "--processes",
        type=int,
        default=_DEFAULT_CORES,
        help="number of processes to use",
    )
    parser.add_argument(
        "-b",
        "--build-root",
        type=Path,
        default=_DEFAULT_BUILD_ROOT,
        help="path to the root of the MATE binary distribution",
    )
    parser.add_argument(
        "-r",
        "--reset",
        action="store_true",
        help="clear the cache before continuing",
    )
    parser.add_argument(
        "--directory",
        type=Path,
        default=None,
        help="directory to search for programs",
    )
    parser.add_argument(
        "-a",
        "--analysis",
        type=Path,
        default=_DEFAULT_ANALYSIS,
        help="pointer analysis binary",
    )
    parser.add_argument(
        "-n",
        "--no-reduce",
        action="store_true",
    )
    parser.add_argument(
        "-i",
        "--interpreter",
        action="store_true",
        help="use the Soufflé interpreter rather than the analysis binary")
    parser.add_argument(
        "program",
        type=Path,
        nargs="?",
        default=None,
        help="path to program to check and reduce",
    )
    return parser


# Should only be constructed by get_config, which checks invariants.
@dataclass(frozen=True)
class Config:
    debug: bool = False
    cores: int = _DEFAULT_CORES
    processes: int = _DEFAULT_CORES
    build_root: Path = _DEFAULT_BUILD_ROOT
    reset: bool = False
    program: Optional[Path] = None
    directory: Optional[Path] = None
    analysis: Path = _DEFAULT_ANALYSIS
    no_reduce: bool = False
    interpreter: bool = False


def check_config(config: Config) -> None:
    if config.cores < config.processes:
        logger.error(
            "The number of processes should be less than the number of cores")
        exit(1)
    if config.cores % config.processes != 0:
        logger.error(
            "The number of processes should divide the number of cores")
        exit(1)


def get_config(cmdline_args: List[str] = argv[1:]) -> Config:
    args = argument_parser().parse_args(cmdline_args)
    config = Config(debug=args.debug,
                    cores=args.cores,
                    processes=args.processes,
                    build_root=args.build_root,
                    reset=args.reset,
                    program=args.program,
                    directory=args.directory,
                    analysis=args.analysis,
                    no_reduce=args.no_reduce,
                    interpreter=args.interpreter)
    check_config(config)
    return config


def main():
    config = get_config()
    logger.addHandler(StreamHandler())
    logger.setLevel(INFO)
    if config.debug:
        logger.setLevel(DEBUG)

    if config.reset and _CACHE.exists():
        rmtree(_CACHE)

    if not _CACHE.exists():
        _CACHE.mkdir()

    program = config.program
    if _CREDUCE.is_file():
        logger.info("Reducing with creduce...")
        program = next(find_programs(dirs=[getcwd()]))
        assert program is not None

    if program is not None:
        exit(go(config, program).value)

    dirs = _DIRS
    if config.directory is not None:
        dirs = [config.directory]

    logger.debug(f"Starting {config.processes} processes...")
    start = time()
    pool = Pool(config.processes)
    pool.map(partial(check_and_show, config), list(find_programs(dirs=dirs)))
    logger.info(f"Done, {int(time() - start)}s elapsed")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
