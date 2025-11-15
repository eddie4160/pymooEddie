"""Plot the initial Latin hypercube population and mark Pareto fronts.

This script replicates the default Eddie NSGA-II scaffold configuration to
produce an initial population using Latin hypercube sampling, evaluates the
ZDT4 objectives, performs non-dominated sorting, and visualizes the fronts.

Run from the repository root, e.g.::

    python Eddie/plot.py --output Eddie/pareto_front.png

You can customize the population size, variable bounds, and RNG seed via the
available command-line options. Use ``--show`` to display the figure
interactively (requires a graphical backend).
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from typing import List, Sequence, Tuple

import matplotlib.pyplot as plt
import numpy as np


@dataclass
class OptimizationParameters:
    """Subset of the C++ optimization parameters needed for plotting."""

    population_size: int = 100
    variable_lower_bounds: Tuple[float, ...] = (0.0, 0.0, 0.0, 0.0, 0.0)
    variable_upper_bounds: Tuple[float, ...] = (1.0, 1.0, 1.0, 1.0, 1.0)
    random_seed: int = 42

    @property
    def dimension(self) -> int:
        return len(self.variable_lower_bounds)


def parse_bounds(value: str, label: str) -> Tuple[float, ...]:
    try:
        parsed = tuple(float(x.strip()) for x in value.split(","))
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"Failed to parse {label}: {exc}") from exc
    return parsed


def latin_hypercube_population(
    params: OptimizationParameters, rng: np.random.Generator
) -> np.ndarray:
    """Generate a Latin hypercube population scaled to parameter bounds."""

    dim = params.dimension
    pop_size = params.population_size

    lhs = np.zeros((pop_size, dim))

    for j in range(dim):
        permuted = rng.permutation(pop_size)
        lhs[:, j] = (permuted + rng.random(pop_size)) / pop_size

    lower = np.asarray(params.variable_lower_bounds)
    upper = np.asarray(params.variable_upper_bounds)
    span = upper - lower

    return lower + lhs * span


def evaluate_zdt4_population(population: np.ndarray) -> np.ndarray:
    """Evaluate ZDT4 objectives for each individual in the population."""

    if population.shape[1] < 2:
        raise ValueError("ZDT4 requires at least two decision variables")

    f1 = population[:, 0]
    remaining = population[:, 1:]

    g = 1.0 + 10.0 * (population.shape[1] - 1)
    if remaining.size:
        g = g + np.sum(remaining**2 - 10.0 * np.cos(4.0 * np.pi * remaining), axis=1)

    h = 1.0 - np.sqrt(f1 / g)
    f2 = g * h

    return np.column_stack((f1, f2))


def dominates(a: Sequence[float], b: Sequence[float]) -> bool:
    """Return True if objective vector ``a`` Pareto-dominates ``b``."""

    better_or_equal = all(x <= y for x, y in zip(a, b))
    strictly_better = any(x < y for x, y in zip(a, b))
    return better_or_equal and strictly_better


def fast_non_dominated_sort(objectives: np.ndarray) -> Tuple[List[List[int]], np.ndarray]:
    """Perform the classic fast non-dominated sorting used by NSGA-II."""

    pop_size = objectives.shape[0]
    S = [set() for _ in range(pop_size)]
    domination_counts = np.zeros(pop_size, dtype=int)
    fronts: List[List[int]] = [[]]

    for p in range(pop_size):
        for q in range(pop_size):
            if p == q:
                continue
            if dominates(objectives[p], objectives[q]):
                S[p].add(q)
            elif dominates(objectives[q], objectives[p]):
                domination_counts[p] += 1
        if domination_counts[p] == 0:
            fronts[0].append(p)

    current_front = 0
    while current_front < len(fronts) and fronts[current_front]:
        next_front: List[int] = []
        for p in fronts[current_front]:
            for q in S[p]:
                domination_counts[q] -= 1
                if domination_counts[q] == 0:
                    next_front.append(q)
        current_front += 1
        if next_front:
            fronts.append(next_front)

    ranks = np.full(pop_size, len(fronts), dtype=int)
    for idx, front in enumerate(fronts):
        for individual in front:
            ranks[individual] = idx + 1

    return fronts, ranks


def plot_fronts(
    objectives: np.ndarray,
    fronts: Sequence[Sequence[int]],
    output_path: str | None,
    show_plot: bool,
) -> None:
    """Plot the objective space and highlight Pareto fronts."""

    if not fronts or not fronts[0]:
        raise ValueError("No Pareto fronts available for plotting")

    cmap = plt.get_cmap("viridis")
    num_fronts = len(fronts)

    fig, ax = plt.subplots(figsize=(8, 6))

    for front_idx, front in enumerate(fronts):
        points = objectives[np.array(front)]
        color = cmap(front_idx / max(1, num_fronts - 1))
        size = 80 if front_idx == 0 else 50
        edgecolor = "black" if front_idx == 0 else "none"
        ax.scatter(
            points[:, 0],
            points[:, 1],
            label=f"Front {front_idx + 1}",
            s=size,
            color=color,
            edgecolors=edgecolor,
            alpha=0.8,
        )

    ax.set_xlabel("Objective 1 (f1)")
    ax.set_ylabel("Objective 2 (f2)")
    ax.set_title("Initial Population Pareto Fronts (ZDT4)")
    ax.legend(title="Pareto Fronts")
    ax.grid(True, linestyle="--", alpha=0.3)

    fig.tight_layout()

    if output_path:
        fig.savefig(output_path, dpi=300)
        print(f"Saved Pareto front plot to {output_path}")

    if show_plot:
        plt.show()
    else:
        plt.close(fig)


def build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--population-size",
        type=int,
        default=OptimizationParameters.population_size,
        help="Number of individuals in the initial population (default: 100)",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=OptimizationParameters.random_seed,
        help="Random seed for Latin hypercube sampling (default: 42)",
    )
    parser.add_argument(
        "--lower-bounds",
        type=str,
        default=",".join(str(x) for x in OptimizationParameters.variable_lower_bounds),
        help=(
            "Comma-separated lower bounds for each variable (default: 0,0,0,0,0). "
            "Length must match the upper bounds."
        ),
    )
    parser.add_argument(
        "--upper-bounds",
        type=str,
        default=",".join(str(x) for x in OptimizationParameters.variable_upper_bounds),
        help=(
            "Comma-separated upper bounds for each variable (default: 1,1,1,1,1). "
            "Length must match the lower bounds."
        ),
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="Optional path to save the generated plot as an image file.",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the plot interactively (may require a GUI backend).",
    )
    return parser


def main() -> None:
    parser = build_argument_parser()
    args = parser.parse_args()

    lower_bounds = parse_bounds(args.lower_bounds, "lower bounds")
    upper_bounds = parse_bounds(args.upper_bounds, "upper bounds")

    if len(lower_bounds) != len(upper_bounds):
        parser.error("Lower and upper bounds must contain the same number of values")

    params = OptimizationParameters(
        population_size=args.population_size,
        variable_lower_bounds=lower_bounds,
        variable_upper_bounds=upper_bounds,
        random_seed=args.seed,
    )

    rng = np.random.default_rng(params.random_seed)
    population = latin_hypercube_population(params, rng)
    objectives = evaluate_zdt4_population(population)
    fronts, ranks = fast_non_dominated_sort(objectives)

    print(f"Generated population with {population.shape[0]} individuals")
    print(f"Identified {len(fronts)} Pareto fronts")
    for idx, front in enumerate(fronts, start=1):
        print(f"  Front {idx}: {len(front)} individuals")

    plot_fronts(objectives, fronts, args.output, args.show)


if __name__ == "__main__":
    main()
