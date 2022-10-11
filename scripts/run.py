#!/usr/bin/env python3

"""Helper to run cclyzer++ and produce human-readable output.

Especially useful for diffing output between two cclyzer++ revisions to see
what has changed. Example use:

    alias ccl-dev-run='docker run --rm -it --mount type=bind,src=$PWD,target=/work --workdir=/work clyzer-dev'
    rm -rf build

    git checkout <old>
    ccl-dev-run cmake -G Ninja -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=1
    ccl-dev-run cmake --build build -j $(nproc)
    git checkout main scripts/run.py
    ccl-dev-run python3 scripts/run.py prog.ll old

    git checkout <new>
    ccl-dev-run cmake -G Ninja -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=1
    ccl-dev-run cmake --build build -j $(nproc)
    ccl-dev-run python3 scripts/run.py prog.ll new old
"""

from __future__ import annotations

import filecmp
import gzip

from os.path import realpath
from pathlib import Path
from subprocess import check_call, run
from sys import argv, exit
from typing import Optional, List

PARENT: Path = Path(realpath(__file__)).parent
BUILD: Path = PARENT.parent / "build"

def die(msg: str) -> None:
    print("[ERR]", msg)
    exit(1)


def run_cclyzerpp(ir_path: Path, out: Path) -> None:
    check_call(
        [
            "opt",
            "-load",
            BUILD / "libSoufflePA.so",
            "-load",
            BUILD / "libPAPass.so",
            "-disable-output",
            "-cclyzer",
            "-debug-datalog=true",
            "-debug-datalog-dir={}".format(out),
            "-context-sensitivity={}".format("1-callsite"),
            ir_path,
        ]
    )


def normalize(out: Path) -> None:
    for relation_path in out.iterdir():
        if relation_path.suffix != ".gz":
            continue
        # remove .gz suffix
        uncompressed = relation_path.with_suffix("")
        with open(uncompressed, "w") as out_file:
            with gzip.open(relation_path, "rt") as in_file:
                # Unfortunately, the FactGenerator outputs relations with
                # duplicates.
                lines = sorted(set(in_file.readlines()))
                for line in lines:
                    out_file.write(line)


# These relations have been renamed at some point in the past, so when
# comparing outputs all of them must be considered.
RENAMED = {
    frozenset({
        "add_instruction",
        "add_instr"
    })
}


def compare_rel(relation_path: Path, compare_path: Path) -> None:
    if not compare_path.exists():
        return
    if relation_path.name.startswith("unification"):
        # Explicitly non-deterministic
        return
    if relation_path.suffix != ".csv":
        return
    same = filecmp.cmp(str(relation_path), str(compare_path))
    if not same:
        print(f"{relation_path} differs from {compare_path}")
        run(["diff", "--unified", str(relation_path), str(compare_path)])


def compare_dirs(out: Path, compare: Path) -> None:
    assert out != compare
    out_rels = list(out.iterdir())
    cmp_rels = list(compare.iterdir())
    if len(out_rels) != len(cmp_rels):
        # In this case, this script might not output all differences.
        print("[WARN] Different number of relations")
    for relation_path in out.iterdir():
        compare_path = compare / relation_path.name
        compare_rel(relation_path, compare_path)
        for names in RENAMED:
            if relation_path.name in names:
                for name in names:
                    compare_path = compare / name
                    compare_rel(relation_path, compare_path)
    for relation_path in compare.iterdir():
        compare_path = out / relation_path.name
        compare_rel(relation_path, compare_path)
        for names in RENAMED:
            if relation_path.name in names:
                for name in names:
                    compare_path = out / name
                    compare_rel(relation_path, compare_path)


def main(argv: List[str]) -> None:
    if len(argv) != 3 and len(argv) != 4:
        die("Usage: ./run.py LLVMBC OUTDIR [COMPAREDIR]")

    ir_path = Path(argv[1])
    if not (ir_path.suffix == ".ll" or ir_path.suffix == ".bc"):
        die("Expected LLVM bitcode with suffix ll or bc")
    if not ir_path.exists():
        die(f"No LLVM program at {ir_path}")

    out = Path(argv[2])
    if not out.is_dir():
        out.mkdir()

    compare: Optional[Path] = None
    if len(argv) == 4:
        compare = Path(argv[3])
    if compare is not None and not compare.is_dir():
        die(f"Expected directory at {compare}")
    if compare is not None and compare == out:
        die("Expected comparison directory to differ from output directory")

    run_cclyzerpp(ir_path, out)
    normalize(out)

    if compare is not None:
        compare_dirs(out, compare)

if __name__ == "__main__":
    main(argv)
