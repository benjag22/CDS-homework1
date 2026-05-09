# Compact Data Structures - Homework 1

## Requirements

- Linux machine
- CMake >= 3.16
- C++ compiler compatible with C++ 23
- Python >= 3.13 and `uv` if you want to plot results

## Compile

```bash
./build.sh
```

## Execute

Two example files are included in this repository under the `texts` directory. You may use them to test this project, or
you can use any other text file in your machine. Test results will be placed under the `data` directory.

Please note that the tests upper bound cannot be greater than `the length of the file - 2`, otherwise you may get
segmentation faults.

```
Usage: ./build/cds-hw1 <text_file_path> [options]

<text_file_path> must be an absolute path

Options:

  -r : Number of runs per test case.
       default: 32
       min: 32

  -l : Lower bound for test cases range.
       default: 1
       min: 1

  -u : Upper bound for test cases range.
       default: 1025
       min: 1

  -s : Step of test cases.
       default: 2
       min: 1

  -m : Whether to use multiplicative steps instead of additive.
       default: false
```

You may run the following to plot the results. Plots will be stored in the `plots` directory.

```bash
uv sync
uv run plotter.py
```
