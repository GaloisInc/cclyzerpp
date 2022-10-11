import csv
import gzip


def test_type_compatible_animals(run):
    out_dir = run("virtual.cpp")
    with gzip.open(out_dir / "type_compatible.csv.gz", "rt") as f:
        compat = set(tuple(row) for row in csv.reader(f, delimiter="\t"))

    # Here's the LLVM:
    #
    # %class.Animal = type { i32 (...)** }
    # %class.Llama = type { %class.Animal }
    # %class.Dog = type { %class.Animal }

    assert ("%class.Animal", "%class.Animal") in compat
    assert ("%class.Dog", "%class.Llama") in compat
    assert ("%class.Llama", "%class.Animal") not in compat


def test_type_compatible_builtin(run):
    out_dir = run("hello.c")
    with gzip.open(out_dir / "type_compatible.csv.gz", "rt") as f:
        compat = set(tuple(row) for row in csv.reader(f, delimiter="\t"))

    assert ("label", "label") in compat
    assert ("metadata", "metadata") in compat
    assert ("void", "void") in compat
    assert ("i32", "i32") in compat
