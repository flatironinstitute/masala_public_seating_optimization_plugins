# Seating Optimization Masala Plugins

## Version

The Seating Optimization Masala Plugins library is currently version 0.1 (alpha).

## Description

Plugins for the Masala software suite that allow Masala to optimize seating arrangements at events like weddings, using Masala classical or quantum optimizers. 

## Author

- Vikram K. Mulligan, Research Scientist, Center for Computational Biology, Flatiron Institute (vmulligan@flatironinstitute.org)

## Setup

1.  Compile Masala using the buildme.sh script in the masala/ directory.
2.  Set up a simlink to the headers/masala/headers directory.  For instance: `ln -s ~/my_projects/masala/headers/ ~/my_projects/seating_optimization_masala_plugins/headers/masala/`
3.  Set up simlinks to the dylib files or so files in your masala build directory.  For instance `cd ~/myprojects/seating_optimization_masala_plugins/lib && ln -s ~/my_projects/masala/build/lib* .`
4.  Set up a simlink to the external/ directory.  For instance: `ln -s ~/my_projects/masala/external ~/my_projects/seating_optimization_masala_plugins/`
5.  Set up a simlink to the code\_templates/ directory.  For instance: `ln -s ~/my_projects/masala/code_templates ~/my_projects/seating_optimization_masala_plugins/`
6.  Run the buildme.sh script (`./buildme.sh`) to compile and link the Masala seating optimization plugin library.

## Unit Testing

To run unit tests, use the following commands after compiling:

./build/seating_optimization_tests`
`./build/registration_tests`

Note that some tests require the following environment variables to be set:

`MASALA_PATH`: This should point to your core Masala repository's `build/` directory.
`MASALA_STANDARD_PLUGINS`: This should point to the (top-level) directory that countains the Masala standard plugins repository.
`MASALA_SEATING_OPTIMIZATION_PLUGINS`: This should point to the top-level directory containing this repository.

## Compilation

- Note that default compilation is in release mode.
- To compile in debug mode, edit cmake/CMakeLists.txt and change `SET( MODE release )` to `SET( MODE debug )`.

## Building Doxygen code documentation

All source code (both manually-written sub-libraries and auto-generated API sub-libraries) is documented with Doxygen tags.  To generate Doxygen HTML documentation, first, follow the instructions above to build Masala's Core library and the Seating Optimization Masala Plugins library.  Second, build Doxygen documentation with:

```
doxygen Doxyfile.src
```

(Note that Doxygen must be installed.) Documentation will be addded to the `html_docs/` directory.  Delete this directory to recompile documentation from scratch.
