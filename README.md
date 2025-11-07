# Static Timing Analysis
## Lexi MacLean

This is a project for EE 5301 VLSI Design automation.

It parses a subset of `.lib` and `.isc` files to find the total delay of the circuit described by the `.isc` file given gate parameters defined by the `.lib` file.

Documentation on usage here will be added later as it conforms to the expectations provided by the class.

## File Descriptions

`parser.cpp`: this file contains the `main` function and handles some basic argument parsing and file i/o

`netlist.h`: struct definitions and function declarations for dealing with the netlist

`netlist.cpp`: implementations for functions regarding the netlist including parsing the `.isc`, and the traversals

`lut.h`: struct definition and function declarations for the LUT

`lut.cpp`: implementations for functions regarding the LUT including parsing the `.lib`

`Makefile`: makefile

`gen_demos.sh`: script to generate demo directory

`demo/`: directory of directories containing outputs of `./parser` with each of the provided benchmark `.isc` files, including timing in `timed.txt`

`README.md`: this file
