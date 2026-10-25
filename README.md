# TwinWidth Solver

This repository contains twin-width solver code developed for thesis work.
It includes a custom greedy solver, Monte Carlo Tree Search variants, and an MD-based adapter built on top of the external `HydraPrime` library.

## Repository structure

- `src/` - main implementation source files
- `inc/` - public headers
- `data/` - input graphs
- `HydraPrime/` - external third-party library used by `MDAdapter`

## Key targets

- `twin_width_solver` - main solver executable

## HydraPrime dependency

The `HydraPrime` directory contains external third-party code used for modular decomposition.
This code is included as an external dependency and is built together with the project.

`HydraPrime` is provided under the same Apache License 2.0 terms as this repository.
See `LICENSE` for the full license text.

Source for `HydraPrime` is available at:

- `https://doi.org/10.5281/zenodo.7996823`
- `https://github.com/TheoryInPractice/hydraprime`

## Build instructions

From the repository root:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## License

This project is distributed under the Apache License, Version 2.0.
The external `HydraPrime` dependency is also licensed under Apache License 2.0.
