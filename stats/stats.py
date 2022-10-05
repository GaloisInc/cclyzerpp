"""Collect performance and precision statistics of the pointer analysis."""
import argparse
import csv
import gzip
import json
import logging
import subprocess
from collections import defaultdict
from enum import Enum, unique
from itertools import chain, tee
from pathlib import Path
from shutil import rmtree
from time import time
from typing import Dict, List, NamedTuple, NewType, Tuple, TypeVar, Union

from plotly.graph_objects import Bar, Figure, Scatter


@unique
class Command(Enum):
    COLLECT: str = "collect"
    REPORT: str = "report"


@unique
class ContextSensitivity(Enum):
    """Valid context sensitivity settings for the pointer analysis."""

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

    def __str__(self) -> str:
        """For inclusion in --help."""
        return self.value


class RelationInfo(NamedTuple):
    name: str
    headers: str


@unique
class Relation(Enum):
    VAR_ALIAS_SIZES = RelationInfo("var_alias_sizes", ["variable", "alias set size"])
    VAR_POINTS_TO_SIZES = RelationInfo("var_points_to_sizes", ["variable", "points-to set size"])
    FUNCTION_OUT_DEGREE = RelationInfo(
        "function_out_degree", ["function name", "function out degree"]
    )
    FUNCTION_IN_DEGREE = RelationInfo("function_in_degree", ["function name", "function in degree"])


_RELATION_BY_NAME: Dict[str, Relation] = {rel.value.name: rel for rel in Relation}


class RunStats(NamedTuple):
    duration: int
    relation_values: Dict[Relation, List[Tuple[str, int]]]


ContextSensitivityStats = NewType(
    "ContextSensitivityStats", Dict[Path, Dict[ContextSensitivity, RunStats]]
)


def run(*args: str, timeout: int = 60 * 30) -> int:
    """Runs a command, returning the number of seconds it took.

    The timeout is given in seconds.
    """
    logging.debug(f"Running '{' '.join(args)}'")
    start = time()
    try:
        completed = subprocess.run(args, capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        logger.error(f"Subprocess timed out: {' '.join(args)}")
    if completed.returncode == 0:
        duration = time() - start
        logging.debug(f"Done, {int(duration)}s elapsed")
        return duration

    logging.error(
        "\n".join(
            [
                f"Error while running command:",
                "command:",
                " ".join(args),
                "stdout:",
                completed.stdout.decode("utf-8", errors="ignore"),
                "stderr:",
                completed.stderr.decode("utf-8", errors="ignore"),
            ]
        )
    )
    exit(1)


def get_relation_values(path: Path, relation: Relation) -> List[Tuple[str, int]]:
    logging.debug(f"Gathering statistics for {relation.value.name}...")

    relation_path = path / f"{relation.value.name}.csv.gz"
    if not relation_path.is_file():
        logging.error(f"Expected a compressed CSV file at {relation_path}.")
        exit(1)

    with gzip.open(relation_path, "rt") as f:
        return [(row[0], int(row[1])) for row in csv.reader(f, delimiter="\t")]


def cant_exist(path: Path):
    if path.exists():
        logging.error(f"{path} exists, refusing to overwrite.")
        exit(1)


def mkdir(path: Path, can_exist: bool = True):
    if not can_exist:
        cant_exist(path)
    if path.is_dir():
        return
    try:
        path.mkdir()
    except Exception:
        logging.exception(f"Failed to create directory at {path}!")
        exit(1)


def run_souffle(
    program: Path,
    souffle_program: Path,
    output_dir: Path,
    fact_generator_exe: str = "FactGenerator",
    souffle_exe: str = "souffle",
    extra_fact_generator_arguments: List[str] = [],
    extra_souffle_arguments: List[str] = [],
    retain_csvs: bool = True,
    timeout: int = 60 * 30,
) -> RunStats:
    logging.info("Generating facts...")
    logging.debug(f"Program: {program}")

    if not output_dir.exists():
        mkdir(output_dir)

    flags_str = "-".join(
        arg.replace("/", "-").replace(" ", "-")
        for arg in chain(extra_fact_generator_arguments, extra_souffle_arguments)
    )
    facts_dir = output_dir / f"{program.name}-facts-{flags_str}"
    cant_exist(facts_dir)

    run(
        fact_generator_exe,
        "--out-dir",
        str(facts_dir),
        str(program),
        *extra_fact_generator_arguments,
        timeout=timeout,
    )

    results_dir = output_dir / f"{program.name}-results-{flags_str}"
    mkdir(results_dir, can_exist=False)

    logging.info("Running Souffle...")
    logging.debug(f"Souffle program: {souffle_program}")
    duration = run(
        souffle_exe,
        str(souffle_program),
        "-F",
        str(facts_dir),
        "-D",
        str(results_dir),
        *extra_souffle_arguments,
        timeout=timeout,
    )

    stats = RunStats(duration, {rel: get_relation_values(results_dir, rel) for rel in Relation})
    if not retain_csvs:
        rmtree(output_dir)
    return stats


def collect_statistics(
    programs: List[Path],
    context_sensitivity: List[ContextSensitivity],
    output_dir: Path,
    souffle_program: Path,
    fact_generator_exe: str = "FactGenerator",
    souffle_exe: str = "souffle",
    extra_souffle_arguments: List[str] = [],
    extra_fact_generator_arguments: List[str] = [],
    retain_csvs: bool = True,
    timeout: int = 60 * 30,
) -> ContextSensitivityStats:
    return ContextSensitivityStats(
        {
            program: {
                config: run_souffle(
                    program,
                    souffle_program,
                    output_dir,
                    fact_generator_exe=fact_generator_exe,
                    souffle_exe=souffle_exe,
                    extra_fact_generator_arguments=["--context-sensitivity", config.value]
                    + extra_fact_generator_arguments,
                    extra_souffle_arguments=extra_souffle_arguments,
                    retain_csvs=retain_csvs,
                    timeout=timeout,
                )
                for config in context_sensitivity
            }
            for program in programs
        }
    )


_STATS_FILENAME: str = "stats.json"


def dump_statistics(path: Path, stats: ContextSensitivityStats) -> None:
    with open(path / _STATS_FILENAME, mode="w") as f:
        json.dump(
            {
                str(program): {
                    cs.value: [
                        run_stats.duration,
                        {
                            rel.value.name: stats
                            for (rel, stats) in run_stats.relation_values.items()
                        },
                    ]
                    for (cs, run_stats) in d.items()
                }
                for (program, d) in stats.items()
            },
            f,
        )


def load_statistics(path: Path) -> ContextSensitivityStats:
    try:
        stats_path = path / _STATS_FILENAME
        with open(stats_path, mode="r") as f:
            blob = json.load(f)
    except FileNotFoundError:
        logging.error(f"Couldn't find statistics at {stats_path}")
        exit(1)
    try:
        return {
            Path(program): {
                ContextSensitivity(config): RunStats(
                    run_stats[0],
                    {_RELATION_BY_NAME[rel]: values for (rel, values) in run_stats[1].items()},
                )
                for (config, run_stats) in d.items()
            }
            for (program, d) in blob.items()
        }
    except IndexError:
        logging.error("Malformed statistics!")
        exit(1)


def compare_bars(
    output: Path,
    title: str,
    groups: Dict[str, List[Tuple[str, int]]],
    normalize: bool = False,
    width: int = 1800,
    height: int = 800,
    xaxis_title: str = "x",
    yaxis_title: str = "y",
):
    """Create a bar graph comparing a few different groups."""
    length = 0
    bars = []
    for (key, data) in groups.items():
        copy1, copy2 = tee(data)
        xs, ys = [row[0] for row in copy1], [row[1] for row in copy2]
        length = len(xs)
        bars.append(Bar(name=key, x=xs, y=ys))
    fig = Figure(data=bars)

    if normalize:
        fig.update_layout(barnorm="fraction")

    fig.update_layout(
        title=title,
        xaxis_title=xaxis_title,
        yaxis_title=yaxis_title,
        width=width,
        height=height,
        xaxis={"type": "category", "categoryorder": "array", "categoryarray": list(range(length))},
    )
    fig.write_image(output)


def compare_scatter(
    output: Path,
    title: str,
    groups: Dict[str, Tuple[int, int]],
    xaxis_title: str = "x",
    yaxis_title: str = "y",
    width: int = 1800,
    height: int = 800,
):
    """Create a scatter plot comparing a small number of distinct points."""
    fig = Figure(
        data=Scatter(
            x=[row[0] for row in groups.values()],
            y=[row[1] for row in groups.values()],
            text=list(groups.keys()),
            textposition="top right",
            # marker={"color": list(groups.keys()), "size": [10 for _ in range(len(groups))]},
            mode="markers+text",
        )
    )
    fig.update_layout(
        title=title, xaxis_title=xaxis_title, yaxis_title=yaxis_title, width=width, height=height
    )

    fig.write_image(output)


def mean(ys: Union[int, float]) -> Union[int, float]:
    return sum(ys) / len(ys)


R = TypeVar("R")
S = TypeVar("S")
T = TypeVar("T")


def curry(d: Dict[Tuple[R, S], T]) -> Dict[R, Dict[S, T]]:
    ret = defaultdict(dict)
    for ((outer_key, inner_key), value) in d.items():
        ret[outer_key][inner_key] = value
    return ret


def uncurry(d: Dict[R, Dict[S, T]]) -> Dict[Tuple[R, S], T]:
    return {
        (outer_key, inner_key): value
        for (outer_key, subdict) in d.items()
        for (inner_key, value) in subdict.items()
    }


def flip(d: Dict[Tuple[R, S], T]) -> Dict[Tuple[S, R], T]:
    return {(second, first): value for ((first, second), value) in d.items()}


def convolute(d: Dict[R, Dict[S, T]]) -> Dict[S, Dict[R, T]]:
    return curry(flip(uncurry(d)))


def report(output: Path, stats: ContextSensitivityStats, raw_plots: bool = False) -> None:
    logging.debug(f"Generating a report from data at {output}...")

    mkdir(output)

    for (program, results) in stats.items():
        logging.debug(f"Generating per-program plots for {program}...")

        program_output_path = output / program.name
        mkdir(program_output_path, can_exist=False)

        # Plots of the raw relation values
        if raw_plots:
            for rel in Relation:
                compare_bars(
                    str((program_output_path / rel.value.name).with_suffix(".svg")),
                    f"{program.name} {rel.value.name}",
                    {
                        context_sensitivity.value: run_stats.relation_values[rel]
                        for (context_sensitivity, run_stats) in results.items()
                    },
                    xaxis_title=f"{rel.value.headers[0]}",
                    yaxis_title=f"{rel.value.headers[1]} (lower is better)",
                )

        # Summary/performance plots
        logging.debug(f"Generating summary/performance plots for {program}...")
        for rel in Relation:
            compare_scatter(
                str((program_output_path / (rel.value.name + "-summary")).with_suffix(".svg")),
                f"Performance and Precision ({program.name})",
                {
                    context_sensitivity.value: (
                        run_stats.duration,
                        mean([row[1] for row in run_stats.relation_values[rel]]),
                    )
                    for (context_sensitivity, run_stats) in results.items()
                },
                xaxis_title="Time in seconds (lower is better)",
                yaxis_title=f"Mean {rel.value.headers[1]} (lower is better)",
            )

    # Wholly generic comparisons across programs
    logging.debug(f"Generating cross-program comparison plots...")
    for rel in Relation:
        convoluted = convolute(stats)
        compare_scatter(
            str((output / (rel.value.name + "-summary-means")).with_suffix(".svg")),
            "Overall Performance and Precision (Sum of Means)",
            {
                context_sensitivity.value: (
                    sum(d.duration for d in paths_to_run_stats.values()),
                    sum(
                        mean([row[1] for row in run_stats.relation_values[rel]])
                        for run_stats in paths_to_run_stats.values()
                    ),
                )
                for (context_sensitivity, paths_to_run_stats) in convoluted.items()
            },
            xaxis_title="Sum of time in seconds (lower is better)",
            yaxis_title=f"Sum of mean {rel.value.headers[1]} (lower is better)",
        )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Gather precision and performance statistics")
    subparsers = parser.add_subparsers(dest="command")
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        dest="verbose",
        default=0,
        help="print verbose logging information",
    )

    collect_parser = subparsers.add_parser(Command.COLLECT.value, help="Collect statistics")
    collect_parser.add_argument("output", type=Path, help="Where to dump collected statistics")
    collect_parser.add_argument(
        "souffle_program",
        type=Path,
        help="Path to the top-level file of the Souffle Datalog program",
    )
    collect_parser.add_argument(
        "program", nargs="+", type=Path, help="LLVM bitcode files to analyze"
    )
    collect_parser.add_argument(
        "--fact-generator-exe",
        type=str,
        default="factgen-exe",
        help="Name/path of the Fact Generator executable",
    )
    collect_parser.add_argument(
        "--souffle-exe", type=str, default="souffle", help="Name/path of the Souffle executable"
    )
    collect_parser.add_argument(
        "--context-sensitivity",
        type=ContextSensitivity,
        choices=list(ContextSensitivity),
        action="append",
        metavar="SENSITIVITY",
        required=True,
        help="Context sensitivity strategies to vary across samples",
    )
    collect_parser.add_argument(
        "--extra-fact-generator-arguments",
        action="append",
        default=[],
        help="Extra arguments to pass to the Fact Generator",
    )
    collect_parser.add_argument(
        "--extra-souffle-arguments",
        action="append",
        default=[],
        help="Extra arguments to pass to Souffle",
    )
    collect_parser.add_argument(
        "--retain-csvs", default=True, help="Whether to keep around pointer analysis results"
    )
    collect_parser.add_argument(
        "--timeout", type=int, default=30, help="Timeout subprocesses after this many minutes"
    )

    report_parser = subparsers.add_parser(Command.REPORT.value, help="Report on statistics")
    report_parser.add_argument("input", type=Path, help="Path to previously collected statistics")
    report_parser.add_argument("output", type=Path, help="Where to put plots")
    report_parser.add_argument(
        "--raw-plots",
        type=bool,
        default=False,
        help="Whether to generate plots of raw data (unhelpful for large programs)",
    )

    return parser


VERBOSITY = [logging.WARNING, logging.INFO, logging.DEBUG]


def main(args: argparse.Namespace) -> None:
    logging.basicConfig(level=VERBOSITY[min(args.verbose, len(VERBOSITY) - 1)])

    if args.command == Command.COLLECT.value:
        dump_statistics(
            args.output,
            collect_statistics(
                [Path(prog) for prog in args.program],
                args.context_sensitivity,
                args.output,
                args.souffle_program,
                fact_generator_exe=args.fact_generator_exe,
                souffle_exe=args.souffle_exe,
                extra_souffle_arguments=args.extra_souffle_arguments,
                extra_fact_generator_arguments=args.extra_fact_generator_arguments,
                retain_csvs=args.retain_csvs,
                timeout=60 * args.timeout,
            ),
        )
        return

    elif args.command == Command.REPORT.value:
        report(args.output, load_statistics(args.input), raw_plots=args.raw_plots)
        return

    assert False, f"Unreachable: {args.command}"


if __name__ == "__main__":
    try:
        main(build_parser().parse_args())
    except Exception as e:
        logging.exception(
            "Uncaught exception! This is definitely a bug, please file an issue. Details:\n\n"
        )
        raise
