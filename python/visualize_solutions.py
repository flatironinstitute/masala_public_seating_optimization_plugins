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
import drawsvg as draw

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
        outlist = filehandle.readlines()
    print( "Read file \"" + filename + "\"." )
    #print( outlist )
    return outlist

## @brief Draw the tables.
def draw_tables( drawing : draw.Drawing, problines : list[str] )->None :
    intables = False
    for fullline in problines :
        line = fullline.strip()
        print(line)
        if intables == False :
            if line == "TABLES:" :
                intables = True
        else :
            if line == "" :
                intables = False
                break
            linesplit = line.split()
            assert len(linesplit) > 2
            if linesplit[0] == "Table_index" :
                continue # Header line
            if linesplit[1] == "CircularTable" :
                assert len(linesplit) == 7
                tableindex = int(linesplit[0])
                x = float(linesplit[2])
                y = float(linesplit[3])
                #angle = float(linesplit[4])
                radius = float(linesplit[6])
                circ = draw.Circle( x, y, radius, stroke_width=0.025, stroke="black", fill="white" )
                drawing.append(circ)
                print( "Parsed a CircularTable with center " + str(x) + ", " + str(y) + " and radius " + str(radius) + "." )
            else :
                raise Exception( "Did not recognize table type \"" + linesplit[1] + "\"." )

## @brief Draw the seats.        
def draw_seats( drawing : draw.Drawing, problines : list[str] )->None :
    inseats = False
    for fullline in problines :
        line = fullline.strip()
        if inseats == False :
            if line == "SEATS:" :
                inseats = True
        else :
            if line == "" :
                inseats = False
                break
            linesplit = line.split()
            assert len(linesplit) == 6
            if linesplit[0] == "Global_seat_index" :
                continue
            seatindex = int(linesplit[0])
            x = float(linesplit[3])
            y = float(linesplit[4])
            angle = float(linesplit[5])
            r1 = draw.Rectangle(x-.225, y-.125, .45, .25, stroke_width=0.025, stroke="black", fill="white" )
            r2 = draw.Rectangle(x-.35, y-.25, .7, .125   , stroke_width=0.025, stroke="black", fill="white" )
            r3 = draw.Rectangle(x-.35, y-.125, .075, .25, stroke_width=0.025, stroke="black", fill="white" )
            r4 = draw.Rectangle(x+.35, y-.125, -.075, .25, stroke_width=0.025, stroke="black", fill="white" )
            g1 = draw.Group( [r1,r2,r3,r4], transform="rotate(" + str(180-angle) + "," + str(x) + "," + str(y) + ") translate(0,-0.2)" )
            drawing.append(g1)
            print( "Parsed a seat with center " + str(x) + ", " + str(y) + " and angle " + str(angle) + " degrees." )

################################################################################
## PROGRAM ENTRY POINT
################################################################################

print( "Starting visualize_solutions.py." )
outprefix, problem_filename, solution_filenames = parse_options()
print( "outprefix =", outprefix )
print( "problem_filename =", problem_filename )
print( "solution_filenames =", solution_filenames )

problines = read_file(problem_filename)
drawing = draw.Drawing( 12, 8, origin="center" )
draw_tables( drawing, problines )
draw_seats( drawing, problines )

drawing.set_pixel_scale(100)
drawing.rasterize()
drawing.save_png( outprefix + ".png" )
#drawing.save_svg( outprefix + ".svg" )
print( "Wrote \"" + outprefix + ".png\"." )