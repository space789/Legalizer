# Legalizer Project

Author: Shiina

This project implements a standard cell legalizer for VLSI design. The legalizer reads placement information in the GSRC Bookshelf format, performs cell legalization to resolve overlaps and align cells to legal positions, and outputs the updated placement in the same format.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Algorithm Overview](#algorithm-overview)
- [Directory Structure](#directory-structure)
- [Requirements](#requirements)
- [Compilation](#compilation)
- [Usage](#usage)
- [Command-Line Arguments](#command-line-arguments)
- [Example](#example)
- [Output Files](#output-files)
- [Notes](#notes)
- [License](#license)

## Introduction
In VLSI design, after global placement, cells may be placed in illegal positions, such as overlapping with other cells or outside the chip boundary. This project aims to legalize the placement of cells by:

- Resolving cell overlaps.
- Aligning cells to the placement sites.
- Minimizing total and maximum displacement from their original positions.

The input and output files follow the GSRC Bookshelf format, making it compatible with standard benchmarks and tools.

## Features
- Reads input files in GSRC Bookshelf format.
- Computes cell density to prioritize legalization.
- Clusters cells based on density and proximity.
- Places cells onto legal sites while minimizing displacement.
- Uses simulated annealing to further optimize placement.
- Outputs the updated placement in GSRC Bookshelf format.

## Algorithm Overview
The legalization process involves the following steps:

1. **Parsing Input Files**: Reads cell information, initial placement, and row (site) definitions from the input files.
2. **Computing Density**: Calculates the density around each cell based on the number of neighboring cells within a specified epsilon distance.
3. **Sorting and Clustering**:
   - Sorts cells based on their computed density.
   - Clusters cells with similar density and proximity to optimize placement order.
4. **Initial Placement**:
   - Places cells onto the nearest legal site that minimizes displacement.
   - Ensures no overlaps occur during initial placement.
5. **Simulated Annealing**:
   - Applies simulated annealing within clusters and globally to further reduce displacement.
   - Swaps cell positions if it leads to a better (lower) total displacement without causing overlaps.
6. **Displacement Calculation**: Calculates the total and maximum displacement after legalization.
7. **Output Generation**: Writes the updated placement and copies necessary files to the output directory in GSRC Bookshelf format.

## Directory Structure
```
project/
├── src/
│   ├── main.cpp
│   ├── Parser.cpp
│   ├── Parser.h
│   ├── Legalizer.cpp
│   ├── Legalizer.h
│   ├── Cell.cpp
│   ├── Cell.h
│   ├── Row.cpp
│   ├── Row.h
│   ├── Site.cpp
│   ├── Site.h
│   ├── Utilities.cpp
│   ├── Utilities.h
│   └── Makefile
├── bench/
│   └── [benchmark directories with input files]
├── output/
│   └── [directories for output files]
└── README.md
```

## Requirements
- C++11 compatible compiler (e.g., GCC 4.8 or higher).
- Standard C++ libraries.

## Compilation
Navigate to the `src` directory and run:

```
make
```
This will compile all source files and produce an executable named `legalizer`.

To clean up object files and the executable, run:

```
make clean
```

## Usage
```
./legalizer INPUT_DIR OUTPUT_DIR [-e double] [-t double]
```
- `INPUT_DIR`: Directory containing the input files in GSRC Bookshelf format.
- `OUTPUT_DIR`: Directory where the output files will be saved.
- `-e double`: (Optional) Sets the epsilon value for density calculation. Default is 10.0.
- `-t double`: (Optional) Sets the timer in minutes for the simulated annealing process. Default is 10.0.

### Help
To display the usage information:
```
./legalizer -h
```
or
```
./legalizer --help
```

## Command-Line Arguments
- `INPUT_DIR` and `OUTPUT_DIR` should be provided without file prefixes. The program will automatically extract the benchmark name from the directory path.

Optional Arguments:
- `-e double`: Sets the epsilon value, which determines the neighborhood radius for density calculation.
- `-t double`: Sets the maximum duration in minutes for the simulated annealing process.

## Example
Assuming you have a benchmark named `toy` located in `../bench/toy/`, and you want to save the output to `../output/toy/`, run:

```
./legalizer ../bench/toy/ ../output/toy/
```
To specify custom epsilon and timer values:

```
./legalizer ../bench/toy/ ../output/toy/ -e 5.0 -t 15.0
```

## Output Files
The program will generate the following files in the `OUTPUT_DIR`:

- `[benchmark].pl`: Updated placement file with legalized cell positions.
- `[benchmark].nodes`: Copied from the input directory.
- `[benchmark].nets`: Copied from the input directory.
- `[benchmark].wts`: Copied from the input directory.
- `[benchmark].scl`: Copied from the input directory.
- `[benchmark].aux`: Copied from the input directory.

Note: `[benchmark]` is automatically determined from the last part of the `INPUT_DIR` path.

## Notes
- **Input Files**: The input directory must contain all necessary GSRC Bookshelf files with consistent naming, such as `toy.aux`, `toy.nodes`, etc.
- **Output Directory**: The program will attempt to create the output directory if it does not exist.
- **Permissions**: Ensure you have read permissions for the input files and write permissions for the output directory.
- **Performance**: The default settings are suitable for small to medium-sized benchmarks. For larger benchmarks, you may need to adjust the epsilon and timer values.
- **Debugging**: Compile with `-DDEBUG_LEGALIZER` to enable debug output in the `Legalizer` class.

## License
This project is released under the MIT License.
