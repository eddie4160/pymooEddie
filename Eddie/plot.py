"""Plot NSGA-II data exported by the Eddie scaffold.

The C++ demo writes ``initial_population.txt`` containing each individual's
decision variables, objective values, and Pareto front rank. This script reads
that ASCII export and visualizes the fronts in objective space.

Example usage::

    ./Eddie/nsga_demo  # generates Eddie/initial_population.txt by default
    python Eddie/plot.py --data Eddie/initial_population.txt --output pareto.png

Use ``--show`` to display the plot interactively instead of writing to disk.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict

import matplotlib.pyplot as plt
import numpy as np

OBJECTIVE_COUNT = 2


def load_population_export(path: Path) -> Dict[str, np.ndarray]:
    """Load the ASCII export produced by ``nsga_demo``.

    The file is expected to contain comment lines beginning with ``#`` followed
    by whitespace-separated columns in the order::

        index front objective_1 objective_2 [variable_1 ... variable_n]

    Returns a dictionary containing the parsed objective matrix, Pareto front
    indices (1-based), and any decision variables for potential downstream use.
    """

    if not path.exists():
        raise FileNotFoundError(f"Population data file not found: {path}")

    data = np.loadtxt(path, comments="#")
    if data.ndim == 1:
        data = data[np.newaxis, :]

    expected_columns = 2 + OBJECTIVE_COUNT
    if data.shape[1] < expected_columns:
        raise ValueError(
            "Population data must contain index, front, and objective columns"
        )

    indices = data[:, 0].astype(int)
    fronts = data[:, 1].astype(int)
    objectives = data[:, 2 : 2 + OBJECTIVE_COUNT]
    variables = data[:, 2 + OBJECTIVE_COUNT :] if data.shape[1] > expected_columns else np.empty((data.shape[0], 0))

    if objectives.shape[1] < 2:
        raise ValueError("At least two objective columns are required for plotting")

    return {
        "indices": indices,
        "fronts": fronts,
        "objectives": objectives,
        "variables": variables,
    }


def plot_fronts(
    objectives: np.ndarray,
    fronts: np.ndarray,
    output_path: str | None,
    show_plot: bool,
) -> None:
    """Plot the objective space and highlight Pareto fronts."""

    if objectives.shape[0] == 0:
        raise ValueError("No objective values available for plotting")

    cmap = plt.get_cmap("viridis")
    unique_fronts = np.unique(fronts)
    num_fronts = unique_fronts.size

    fig, ax = plt.subplots(figsize=(8, 6))

    first_front = unique_fronts.min()
    for plot_idx, front_id in enumerate(unique_fronts):
        mask = fronts == front_id
        if not np.any(mask):
            continue
        points = objectives[mask]
        color = cmap(plot_idx / max(1, num_fronts - 1))
        size = 80 if front_id == first_front else 50
        edgecolor = "black" if front_id == first_front else "none"
        ax.scatter(
            points[:, 0],
            points[:, 1],
            label=f"Front {front_id}",
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
        "--data",
        type=Path,
        default=Path("Eddie/initial_population.txt"),
        help="ASCII file produced by nsga_demo containing objectives and fronts",
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="Path to save the generated plot (omit to skip saving)",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the plot interactively (may require a GUI backend)",
    )
    return parser


def main() -> None:
    parser = build_argument_parser()
    args = parser.parse_args()

    population_data = load_population_export(args.data)
    objectives = population_data["objectives"]
    fronts = population_data["fronts"]

    plot_fronts(objectives, fronts, args.output, args.show)

if __name__ == "__main__":
    main()
