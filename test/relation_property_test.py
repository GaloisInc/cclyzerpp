import csv
import gzip
from enum import Enum, unique
from itertools import chain
from typing import Any, Callable, FrozenSet, NamedTuple, Tuple, Type, TypeVar, Union

import pytest


@unique
class BinaryRelationProperty(Enum):
    FUNCTIONAL = "functional"  # AKA many-to-one
    INJECTIVE = "injective"  # AKA one-to-many
    REFLEXIVE = "reflexive"
    SYMMETRIC = "symmetric"
    TRANSITIVE = "transitive"


# The base types in Souffle, as translated to Python
SouffleType = Union[Type[int], Type[float], Type[str]]
Row = Tuple[SouffleType, ...]

# A projection from an (n + m)-ary relation into a relation between n-tuples
# and m-tuples
Split = Tuple[Tuple[Any, ...], Tuple[Any, ...]]
Projection = Union[
    Callable[[Any], Split],
    Callable[[Any, Any], Split],
    Callable[[Any, Any, Any], Split],
    Callable[[Any, Any, Any, Any], Split],
    Callable[[Any, Any, Any, Any, Any], Split],
]

T = TypeVar("T")


def binary(row0: SouffleType, row1: SouffleType) -> Tuple[Row, Row]:
    return ((row0,), (row1,))


class RelationSpec(NamedTuple):
    """A specification of a few important properties of a relation.

    Because most easy-to-conceptualize relation properties only apply to binary relations, each
    group of properties also specifies a grouping/projection function.
    """

    name: str
    types: Tuple[SouffleType, ...]
    properties: FrozenSet[Tuple[Projection, FrozenSet[BinaryRelationProperty]]]


# Some common sets of properties
_ONE_TO_ONE: FrozenSet[BinaryRelationProperty] = frozenset(
    {BinaryRelationProperty.FUNCTIONAL, BinaryRelationProperty.INJECTIVE}
)
_EQUIVALENCE_RELATION: FrozenSet[BinaryRelationProperty] = frozenset(
    {
        BinaryRelationProperty.REFLEXIVE,
        BinaryRelationProperty.SYMMETRIC,
        BinaryRelationProperty.TRANSITIVE,
    }
)


_SPECS: FrozenSet[RelationSpec] = frozenset(
    {
        RelationSpec(
            name="actual_arg",
            types=(str, int, str),
            properties=frozenset(
                {(lambda i, n, a: ((i, n), (a,)), frozenset({BinaryRelationProperty.FUNCTIONAL}))}
            ),
        ),
        RelationSpec(
            name="subset_aliases.alloc_aliases",
            types=(str, str),
            properties=frozenset({(binary, _EQUIVALENCE_RELATION)}),
        ),
        RelationSpec(
            name="subset_subobjects.alloc_subregion_at_array_index",
            types=(str, int, str),
            properties=frozenset(
                {(lambda a, i, s: ((a, i), (s,)), frozenset({BinaryRelationProperty.INJECTIVE}))}
            ),
        ),
        RelationSpec(
            name="subset_subobjects.alloc_subregion_at_array_index",
            types=(str, int, str),
            properties=frozenset({(lambda a, i, s: ((a, i), (s,)), _ONE_TO_ONE)}),
        ),
        RelationSpec(
            name="subset_subobjects.alloc_subregion_at_field",
            types=(str, int, str),
            properties=frozenset({(lambda a, i, f: ((a, i), (f,)), _ONE_TO_ONE)}),
        ),
        RelationSpec(
            name="subset_subobjects.alloc_subregion_at_path",
            types=(str, str, str),
            properties=frozenset(
                {(lambda a, p, s: ((a, p), (s,)), frozenset({BinaryRelationProperty.FUNCTIONAL}))}
            ),
        ),
        RelationSpec(
            name="array_type_has_component",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="allocation_size",
            types=(str, int),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="func_by_location",
            types=(str, str),
            properties=frozenset({(binary, _ONE_TO_ONE)}),
        ),
        RelationSpec(
            name="func_name",
            types=(str, str),
            properties=frozenset({(binary, _ONE_TO_ONE)}),
        ),
        RelationSpec(
            name="func_returns_value",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.INJECTIVE}))}),
        ),
        RelationSpec(
            name="func_type_nparams",
            types=(str, int),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="func_type_param",
            types=(str, int, str),
            properties=frozenset(
                {(lambda t, i, p: ((t, i), (p,)), frozenset({BinaryRelationProperty.FUNCTIONAL}))}
            ),
        ),
        RelationSpec(
            name="func_type_return",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="sized_alloc_instr",
            types=(str, int),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="struct_type_has_name",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="struct_type_field",
            types=(str, int, str),
            properties=frozenset(
                {(lambda s, i, f: ((s, i), (f,)), frozenset({BinaryRelationProperty.FUNCTIONAL}))}
            ),
        ),
        RelationSpec(
            name="struct_type_nfields",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="type_compatible",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.REFLEXIVE}))}),
        ),
        RelationSpec(
            name="type_has_size",
            types=(str, int),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="variable_has_name",
            types=(str, str),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        RelationSpec(
            name="vector_type_has_size",
            types=(str, int),
            properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        ),
        # TODO(SM): This should obey BinaryRelationProperty.FUNCTIONAL, but something is broken.
        # RelationSpec(
        #    name="static_allocation_type",
        #    types=(str, str),
        #    properties=frozenset({(binary, frozenset({BinaryRelationProperty.FUNCTIONAL}))}),
        # ),
    }
)


def test_relation_properties(run, every_program, context_sensitivity):
    (program, cflags) = every_program

    out_files = run(
        program, context_sensitivity=context_sensitivity.value, additional_cflags=cflags
    )

    for spec in _SPECS:
        print("~" * 40, "Examining", spec.name)  # only printed when the test fails
        with gzip.open(f"{out_files / spec.name}.csv.gz", "rt") as f:
            for (projection, properties) in spec.properties:
                tuples = set()
                for row in csv.reader(f, delimiter="\t"):
                    (group1, group2) = projection(*row)
                    (group1_types, group2_types) = projection(*spec.types)
                    tuples.add(
                        (
                            tuple(ty(elem) for (ty, elem) in zip(group1_types, group1)),
                            tuple(ty(elem) for (ty, elem) in zip(group2_types, group2)),
                        )
                    )
                both = frozenset(chain.from_iterable(tuples))

                for prop in properties:
                    if prop is BinaryRelationProperty.FUNCTIONAL:
                        for (fst, snd) in tuples:
                            assert {pair for pair in tuples if pair[0] == fst} == {(fst, snd)}

                    elif prop is BinaryRelationProperty.INJECTIVE:
                        for (fst, snd) in tuples:
                            assert {pair for pair in tuples if pair[1] == snd} == {(fst, snd)}

                    elif prop is BinaryRelationProperty.REFLEXIVE:
                        for element in both:
                            assert (element, element) in tuples

                    elif prop is BinaryRelationProperty.SYMMETRIC:
                        for (fst, snd) in tuples:
                            assert (snd, fst) in tuples

                    elif prop is BinaryRelationProperty.TRANSITIVE:
                        # Taking one step towards the transitive closure adds no new tuples
                        new_tuples = set((x, w) for x, y in tuples for q, w in tuples if q == y)
                        for pair in new_tuples:
                            assert pair in tuples

                    else:
                        assert False, "Unreachable"
