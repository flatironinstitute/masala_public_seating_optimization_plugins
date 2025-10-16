################################################################################
##    Seating Optimization Masala Pugins
##    Copyright (C) 2025 Vikram K. Mulligan
##
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU Affero General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU Affero General Public License for more details.
##
##    You should have received a copy of the GNU Affero General Public License
##    along with this program.  If not, see <https://www.gnu.org/licenses/>.
################################################################################

## @file python/visualize_solutions.py
## @brief Draws seating diagrams from optimization problem descriptions and solution files.
## @details Usage is: python3 python/visualize_solutiuons.py <output_prefix> <problem_filename> <solution_filename1> <solution_filename2> ...
## @author Vikram K. Mulligan

################################################################################
## IMPORTS
################################################################################

import argparse

################################################################################
## FUNCTION DEFINITIONS
################################################################################

## @brief Parse the commandline options.
def parse_options()->tuple[str, str, list[str]]:
    parser = argparse.ArgumentParser( prog="visualize_solutions", \
        description="An application for drawing seating diagrams from problem definition files and solution files." \
    )
    parser.add_argument( "output_prefix", action="store" )
    parser.add_argument( "problem_filename", action="store" )
    parser.add_argument( "solution_filenames", action="append", nargs="+" )
    args = parser.parse_args()

    return args.output_prefix, args.problem_filename, args.solution_filenames[0]

## @brief Read the problem file.
def read_file( filename : str )->list[str] :
    with open(filename) as filehandle:
        outlist = filehandle.readlines
    print( "Read file \"" + filename + "\"." )
    return outlist

################################################################################
## PROGRAM ENTRY POINT
################################################################################

print( "Starting visualize_solutions.py." )
outprefix, problem_filename, solution_filenames = parse_options()
print( "outprefix =", outprefix )
print( "problem_filename =", problem_filename )
print( "solution_filenames =", solution_filenames )

problines = read_file(problem_filename)