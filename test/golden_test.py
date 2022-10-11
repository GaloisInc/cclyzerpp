import filecmp
import gzip
import os
from itertools import product
from time import sleep
from typing import Final, List, Tuple

import pytest

# These are the relations exported by PAInterface.
_GOLDEN_RELATIONS: Final[List[str]] = [
    "allocation_size",
    "subset.callgraph.callgraph_edge",
    "unification.callgraph.callgraph_edge",
    "global_allocation_by_variable",
    "heap_allocation_by_instr",
    "heap_allocation_by_type_instr",
    "subset.operand_points_to",
    "subset.ptr_points_to",
    "stack_allocation_by_instr",
    "stack_allocation_by_type_instr",
    "user_options",
    "subset.var_points_to",
    "subset_lift.alloc_may_alias_ctx",
    "subset_lift.alloc_must_alias_ctx",
    "subset_lift.alloc_subregion_ctx",
]

_PROGRAMS: Final[List[str]] = [
    "allocation-sizes.c",
    "cfg-test.c",
    "cxxbasic.cpp",
    "functiontable.c",
    "notes.c",
    "ovington-extra-minimal.cpp",
    "points-to_context.c",
    "points-to_malloc-context.c",
    "points-to_new-context.cpp",
    "recurse.c",
    "type-backprop.c",
    "virtual.cpp",
]
_CFLAGS: Final[List[Tuple[str]]] = [("-O0",), ("-O1",)]
_VARIANTS: Final[List[str]] = ["subset", "unification"]
_SENSITIVITIES: Final[List[str]] = ["1-callsite", "2-callsite"]
_INPUTS = list(product(_PROGRAMS, _CFLAGS, _SENSITIVITIES))


@pytest.mark.parametrize("program, cflags, sensitivity", _INPUTS)
def test_pointer_analysis_golden(golden, run, program, cflags, sensitivity):
    gold = golden(
        program, _GOLDEN_RELATIONS, context_sensitivity=sensitivity, additional_cflags=cflags
    )
    assert len(gold) == len(_GOLDEN_RELATIONS)  # sanity check

    out_dir = run(program, context_sensitivity=sensitivity, additional_cflags=cflags)
    for relation in _GOLDEN_RELATIONS:
        out_path = f"{out_dir / relation}.csv.gz"

        # HACK(726): Sometimes, readlines() returns [] here. Not sure why.
        out_lines: List[str] = []
        for _ in range(5):
            with gzip.open(out_path, "rt") as f:
                out_lines = sorted(f.readlines())
            if out_lines != []:
                break
            sleep(1)

        golden_file_name = str(gold[relation])
        assert golden_file_name.endswith(".golden.csv")
        actual_file_name = (
            golden_file_name[: len(golden_file_name) - len(".golden.csv")] + ".actual.csv"
        )

        if os.path.exists(actual_file_name):
            os.remove(actual_file_name)

        with open(actual_file_name, mode="w") as f:
            f.writelines(out_lines)

        assert filecmp.cmp(actual_file_name, golden_file_name), actual_file_name.split("/")[-1]
