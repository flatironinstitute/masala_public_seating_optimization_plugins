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

## Input file format

Example input files are in the `test_data` subdirectory.  Input files allow users to define a seating optimization problem, which consists of the following seating optimization elements:

- The _Room_ defines the space in which tables are laid out.  It is intended for visualization purposes.
- _Seats_ hold Guests.  They are arranged around _Tables_.
- _Guests_ are the people who must be seated.
- _Restraints_ limit particular guests to certain seats.  They simplify the optimization problem by eliminating certain assignments completely.
- _Constraints_ give bonuses or penalties for seating guests in certain seats, or pairs of guests in certain seats in relation to one another.

More information on the syntax for defining each of these is found below.

### Defining the Room

The `Room` class is a pure virtual base class.  Derived classes define rooms of particular shapes.  Currently, the only shape implemented is a rectangular room, implemented in the `RectangularRoom` class.  The syntax for defining a `RectangularRoom` is as follows:

```
RectangularRoom <CENTRE_X> <CENTRE_Y> <ANGLE_DEGREES> <LENGTH> <WIDTH>
```

For instance, to define a room centred at (1.5,2.3), with a length (y dimension) of 5 meters and a width (x dimension) of 9 meters, rotated by 10 degrees clockwise, one would write the following:

```
RectangularRoom 1.5 2.3 10.0 5.0 9.0
```

Only a single `Room` may be defined for a given seating optimization problem.

### Defining Seats and Tables

The `Table` class is a pure virtual base class.  Derived classes define tables of particular shapes.  Currently, only the `CircularTable` class is implemented, with input syntax as follows:

```
CircularTable <CENTRE_X> <CENTRE_Y> <ANGLE_DEGREES> <RADIUS> <SEAT_COUNT> <OPTIONAL_SEAT_LOCAL_INDEX_TO_OMIT_1> <OPTIONAL_SEAT_LOCAL_INDEX_TO_OMIT_2> <OPTIONAL_SEAT_LOCAL_INDEX_TO_OMIT_3> ...
```

Although the table itself is circular, it may be rotated to shift the seats around.  In the unrotated case, seats are numbered from the top, clockwise around the table.  Rotation shifts the seats clockwise.  As an example, the following defines a circular table with four seats, located at (1.0, -1.0), with a radius of 1.5 meters, and with four evenly-spaced seats, the southernmost of which is omitted, and with all seats rotated 15 degrees clockwise:

```
CircularTable 1.0 -1.0 15.0 1.5 4 2
```

Note that the local indices of the seats around the table are, clockwise from the northernmost, 0, 1, 2 and 3.  The arguments for omitting seats are optional: by default, no seats are omitted.

Standalone `Seat` objects, with no associated `Table`, may also be defined using the following syntax:

```
Seat <CENTRE_X> <CENTRE_Y> <ANGLE_DEGREES>
```

A seating optimization problem may have as many tables and seats as the user wishes.  Each guest will be assigned one seat, which means that if there are more seats than there are guests, some seats will remain unassigned.

### Defining Guests

A seating optimization problem must have at least one `Guest` (and a nontrivial problem will have many).  A `Guest` is a person who must be assigned a seat.  Note that the name "guest" is not meant to imply a relationship to the event: in the case of wedding seating optimization, the bride(s) and/or the groom(s) are considered "guests" along with all the people whom they invite to their wedding.

A `Guest` must be assigned a short name string containing no whitespace, used for subsequent setup of constraints, and a full name that may contain whitespace, used for user-facing outputs.  The syntax for defining a `Guest` is:

```
Guest <SHORT_GUEST_NAMESTRING> <FULL_NAME_1> <FULL_NAME_2> <FULL_NAME_3> ...
```

For instance, to define a guest named "John Jacob Jingleheimer Schmidt", we would write:

```
Guest jjj_schmidt John Jacob Jingleheimer Schmidt
```

A seating optimization problem may have any number of guests.  Note that if there are more guests than seats, some seats will inevitably be occupied by more than one guest, so a sensible problem has at least as many seats as guests.

### Defining Restraints

A `Restraint` is are hard prohibitions on assigning a particular seat to a particular guest.  Since that particular seat assignment isn't even considered, a `Restraint` simplifies the problem, making it easier for a solver to optimize.  The `Restraint` class is a pure virtual base class with two derived classes: `RestrictGuestToSeatsRestraint` (which prohibits consideration of any seat but for those listed for a particular guest) and `RestrictGuestToTableRestraint` (which prohibits consideration of any seat but those at the table indicated for a particular guest).

The syntax for a `RestrictGuestToSeatsRestraint` is:

```
RestrictGuestToSeatsRestraint <SHORT_GUEST_NAMESTRING> <SEAT_GLOBAL_INDEX_1> <OPTIONAL_SEAT_GLOBAL_INDEX_2> <OPTIONAL_SEAT_GLOBAL_INDEX_3> ...
```

Global seat indices are zero-based and based on the order in which seats or tables were defined.  The short guest namestring must have previously been defined as part of a `Guest` setup line.

The syntax for a `RestrictGuestToTableRestraint` is:

```
RestrictGuestToTableRestraint <SHORT_GUEST_NAMESTRING> <TABLE_INDEX>
```

The table index is zero-based (matching the order in which tables were defined), and as before, the short guest namestring must have previously been defined as part of a `Guest` setup line.