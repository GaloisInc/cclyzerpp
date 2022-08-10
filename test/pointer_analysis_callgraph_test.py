import csv
import gzip

import pytest


@pytest.mark.skip(reason="Flaky in Gitlab CI")
def test_callgraph_function_pointer(run, context_sensitivity, optimization_flags):
    out_dir = run(
        "unused_argument.c",
        context_sensitivity=context_sensitivity.value,
        additional_cflags=optimization_flags,
    )
    with gzip.open(out_dir / "subset.callgraph.callgraph_edge.csv.gz", "rt") as f:
        edges = {(caller, callee) for (_, callee, _, caller) in csv.reader(f, delimiter="\t")}
    called_from_main = {callee for (caller, callee) in edges if ":main:" in caller}
    assert "<unused_argument.c>:noargs" in called_from_main
    assert "<unused_argument.c>:withargs" in called_from_main
