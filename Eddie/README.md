# Eddie NSGA-II C++ Scaffold

This folder contains a minimal C++ scaffold that you can extend into a full NSGA-II workflow. The goal is to provide a starting
point for integrating ANSYS Fluent evaluations in an optimization loop while keeping the components modular.

## Repository layout

- `main.cpp` – hosts the `main` entry point, reads runtime parameters, generates a Latin hypercube initial population, evaluates
  the selected test problem, and exports an ASCII report for plotting.
- `parameter.h` / `parameter.cpp` – define and populate the `OptimizationParameters` struct by parsing a simple text-based
  configuration file.
- `initpop.*` – Latin hypercube sampling utilities for constructing an initial population.
- `problem.*` – problem definitions (currently ZDT4) and helpers for dispatching objective evaluations.
- `sort.*` – fast non-dominated sorting implementation used to rank the sampled population.
- `plot.py` – convenience script that visualizes the exported population and Pareto fronts.
- `input.txt` – example configuration file consumed by `nsga_demo`.

You can add additional source files (e.g., crossover, mutation, selection operators) next to these files.

## Building the example

A dedicated Makefile is provided. From the repository root run:

```bash
make -C Eddie
```

This builds an executable named `nsga_demo` in the `Eddie/` directory.

## Running

Execute the program by supplying a parameter file (and optionally an output path for the ASCII export):

```bash
./Eddie/nsga_demo Eddie/input.txt Eddie/initial_population.txt
```

If you omit the output file, the default `Eddie/initial_population.txt` is used. When no parameter file is supplied, the program
falls back to `Eddie/input.txt` and prints a warning.

## Parameter file format

The parser accepts a simple key-value format with optional `#` comments. The example `Eddie/input.txt` shows every supported
field. Required entries include the problem name, variable metadata, objective metadata, and evolutionary operator settings. Key
highlights:

- `problem_name` selects the evaluation routine (currently `ZDT4`; `CFD` is reserved for future integration).
- `num_variables`, `variable_names`, `variable_lower_bounds`, `variable_upper_bounds` describe the decision-space bounds.
- `num_objectives` and `objective_names` label the objectives for reporting.
- `parent_population_size`, `offspring_population_size`, `max_generation`, `crossover_probability`,
  `crossover_distribution_index`, `mutation_probability`, `mutation_distribution_index`, and `random_seed` configure the NSGA-II
  operators.

Unrecognized keys trigger warnings so you can spot typos quickly. The program also warns if the offspring population size differs
from the parent population size or if a referenced problem is not implemented yet.

## Plotting

After running `nsga_demo`, visualize the exported population via:

```bash
python Eddie/plot.py --data Eddie/initial_population.txt --output Eddie/pareto_front.png
```

This renders each Pareto front in a different color, making it easy to inspect the initial sampling quality.

## Adding files to the repository with Git

When you create new C++ source files (for example `Eddie/crossover.cpp`), stage them so they become part of the repository
history:

```bash
git add Eddie/crossover.cpp
```

To stage the existing scaffold after making edits:

```bash
git add Eddie/main.cpp Eddie/parameter.cpp Eddie/parameter.h
```

Check your changes:

```bash
git status
```

Finally commit them with an informative message:

```bash
git commit -m "Describe your change here"
```

Push the branch and open a pull request when you are ready to share your updates.

## Next steps

- Implement NSGA-II operators (selection, crossover, mutation).
- Connect the evaluation step to ANSYS Fluent input/output files.
- Extend `problem.cpp` with additional benchmark problems or your CFD interface.
- Add serialization for run results or checkpoints.

Feel free to expand this README with build scripts or usage notes as the project grows.
