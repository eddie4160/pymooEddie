# Eddie NSGA-II C++ Scaffold

This folder contains a minimal C++ scaffold that you can extend into a full NSGA-II workflow. The goal is to provide a starting point for integrating ANSYS Fluent evaluations in an optimization loop.

## Repository layout

- `main.cpp` – hosts the `main` entry point and prints the current optimization parameters. This is where you can eventually integrate the evolutionary loop.
- `parameter.h` – defines the `OptimizationParameters` struct that centralizes all tunable NSGA-II settings (population size, mutation rate, etc.).

You can add additional source files (e.g., crossover, mutation, selection operators) next to these files.

## Building the example

Use any C++17 capable compiler. A simple command with `g++` looks like this:

```bash
g++ -std=c++17 -O2 Eddie/main.cpp -o nsga2
```

From the repository root (`/workspace/pymooEddie` inside the container) this builds an executable named `nsga2`.

## Running

Once compiled, execute the program to see the default configuration that is currently hard-coded in `main.cpp`:

```bash
./nsga2
```

This is a convenient sanity check before you start integrating Fluent or adding the evolutionary operators.

## Customizing parameters

Update the fields inside `Eddie/parameter.h` or extend `main.cpp` to read configuration files. You can also implement the declared `load_parameters_from_file` function to populate the structure from disk.

## Adding files to the repository with Git

When you create new C++ source files (for example `Eddie/population.cpp`), make sure to stage them so they become part of the repository history.

```bash
git add Eddie/population.cpp
```

To stage the existing scaffold after making edits:

```bash
git add Eddie/main.cpp Eddie/parameter.h
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
- Add serialization for parameters to avoid recompiling when tuning hyper-parameters.

Feel free to expand this README with build scripts or usage notes as the project grows.
