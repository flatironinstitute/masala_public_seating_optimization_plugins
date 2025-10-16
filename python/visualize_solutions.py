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
from math import sqrt

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

## @brief Draw the tables.  Return a vector of table centre coordinates.
def draw_tables( drawing : draw.Drawing, problines : list[str] )->list[tuple[float,float]] :
    intables = False
    table_coords = []
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
                table_coords.append((x,y))
                print( "Parsed a CircularTable with center " + str(x) + ", " + str(y) + " and radius " + str(radius) + "." )
            else :
                raise Exception( "Did not recognize table type \"" + linesplit[1] + "\"." )
    return table_coords

## @brief Draw the seats, and return the seat coordinates.
def draw_seats( drawing : draw.Drawing, problines : list[str] )->list[tuple[float,float]] :
    inseats = False
    outlist = []
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
            outlist.append((x,y))
            print( "Parsed a seat with center " + str(x) + ", " + str(y) + " and angle " + str(angle) + " degrees." )
    return outlist

## @brief Add guests' names to the diagram.
def label_guests( drawing : draw.Drawing, soln_lins : list[str], seat_coords : list[tuple[float,float]], table_coords : list[tuple[float,float]] )->None :
    for fullline in soln_lins :
        line = fullline.strip()
        if line == "" :
            continue
        linesplit = line.split( sep="\t" )
        assert len(linesplit) == 6
        if linesplit[0] == "Guest_index" :
            continue
        guestname = linesplit[2].replace("\"", "").replace(" ", "\n")
        seatindex = int(linesplit[3])
        tableindex = int(linesplit[4])
        xseat,yseat = seat_coords[seatindex]
        xtable,ytable = table_coords[tableindex]
        xdiff = xtable-xseat
        ydiff = ytable-yseat
        l = sqrt( xdiff*xdiff + ydiff*ydiff )
        x = 0.5*xdiff/l + xseat
        y = 0.5*ydiff/l + yseat
        t1 = draw.Text( guestname, font_size=.15, x=x, y=y, center=True, style='text-anchor:middle; dominant-baseline:middle;')
        drawing.append(t1)

################################################################################
## PROGRAM ENTRY POINT
################################################################################

print( "Starting visualize_solutions.py." )
outprefix, problem_filename, solution_filenames = parse_options()
print( "outprefix =", outprefix )
print( "problem_filename =", problem_filename )
print( "solution_filenames =", solution_filenames )

problines = read_file(problem_filename)

for i in range(len(solution_filenames)) :
    drawing = draw.Drawing( 12, 8, origin="center" )
    table_coords = draw_tables( drawing, problines )
    seat_coords = draw_seats( drawing, problines )
    soln_lins = read_file(solution_filenames[i])
    label_guests( drawing, soln_lins, seat_coords, table_coords )

    drawing.set_pixel_scale(100)
    drawing.rasterize()
    input_index_str = str( i ).zfill(6)
    drawing.save_png( outprefix + "_" + input_index_str + ".png" )
    #drawing.save_svg( outprefix + "_" + input_index_str + ".svg" )
    print( "Wrote \"" + outprefix + "_" + input_index_str + ".png\"." )