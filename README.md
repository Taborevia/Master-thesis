# Twin-Width Solver

A C++ solver for the **twin-width** graph parameter, developed as part of a master's thesis.

The project implements a Monte Carlo Tree Search (MCTS) solver and a greedy solver. It also supports modular decomposition through the bundled [HydraPrime](https://github.com/TheoryInPractice/hydraprime) library.

## Features

- MCTS-based twin-width solver
- Greedy solver
- Optional greedy simulations and ranking-based move selection
- Adjacency-list and adjacency-matrix graph representations
- Optional modular decomposition via HydraPrime
- Command-line interface for configuring solver parameters and runtime

## Build

### Requirements

- CMake 3.14 or newer
- GCC with C++20 support (the bundled HydraPrime build uses GCC-specific options)
- Ninja, if using the build commands below

From the repository root:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

This builds the `twin_width_solver` executable.

## Usage

Run the solver on a graph instance:

```bash
./build/twin_width_solver --file data/exact_030.gr --resources 10
```

On Windows, the executable is typically located at:

```powershell
.\build\twin_width_solver.exe --file .\data\exact_030.gr --resources 10
```

The program prints the computed twin-width to standard output. MCTS uses a time-based budget and a randomly generated seed, so results may vary between runs.

Show all available command-line options:

```bash
./build/twin_width_solver --help
```

Run the greedy solver:

```bash
./build/twin_width_solver --file data/exact_030.gr --greedy_solver
```

The solver also supports modular decomposition, greedy simulations, ranking, and graph representation options. Run `--help` for the full list and defaults.

## Input format

Input files use a simple edge-list format. The first line contains the number of vertices and edges:

```text
p tww <number_of_vertices> <number_of_edges>
```

Each following line contains one edge as a pair of **1-based** vertex identifiers:

```text
1 2
1 3
2 4
```

A sample instance is available at [`data/exact_030.gr`](data/exact_030.gr).

## Repository structure

- `src/` — solver implementation and command-line entry point
- `inc/` — C++ headers
- `data/` — sample graph instances
- `HydraPrime/` — bundled third-party library used for modular decomposition

## Third-party software and license

HydraPrime is a third-party project. Its source is available at
[TheoryInPractice/hydraprime](https://github.com/TheoryInPractice/hydraprime)
and its published version is available via
[DOI 10.5281/zenodo.7996823](https://doi.org/10.5281/zenodo.7996823).

This project and its bundled dependency (HydraPrime) are licensed under the Apache License 2.0. See [LICENSE](LICENSE.txt) for details.