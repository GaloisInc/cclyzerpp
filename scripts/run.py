#!/usr/bin/env bash

"""Helper to run cclyzer++ and produce human-readable output."""

from __future__ import annotations

import filecmp
import gzip

from os.path import realpath
from pathlib import Path
from subprocess import check_call
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
                for line in sorted(in_file.readlines()):
                    out_file.write(line)


def compare_dirs(out: Path, compare: Path) -> None:
    for relation_path in out.iterdir():
        if relation_path.name != "unification.unify":
            # Explicitly non-deterministic
            continue
        if relation_path.suffix != ".csv":
            continue
        compare_path = compare / relation_path.name
        assert filecmp.cmp(str(relation_path), str(compare_path)), f"{relation_path} differs from {compare_path}"


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

    run_cclyzerpp(ir_path, out)
    normalize(out)

    if compare is not None:
        compare_dirs(out, compare)

if __name__ == "__main__":
    main(argv)
