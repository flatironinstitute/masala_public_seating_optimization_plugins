/*
    Seating Optimization Masala Pugins
    Copyright (C) 2025 Vikram K. Mulligan

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/// @file src/applications/applications_apps/optimize_seating.cc
/// @brief An application that reads an description of a seating optimization problem and solves it using
/// a Masala CFN solver defined in another Masala plugin library.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Registration headers:
#include <registration_api/register_library.hh>

// Masala base headers:
#include <base/types.hh>
#include <base/error/ErrorHandling.hh>
#include <base/managers/tracer/MasalaTracerManager.hh>
#include <base/utility/container/container_util.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// STL headers:
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <getopt.h>
#include <iostream>

using masala::base::Size;
using masala::base::Real;

// Help/usage message for program:

void 
print_help() {
	std::stringstream ss;
	ss << 
		"Below are the options for this program.\n"
		// TODO TODO TODO
	;
}

// Program entry point:
int 
main(
	int argc,
	char * argv[]
) {
    using namespace masala::base::managers::tracer;
    using namespace masala::base::utility::container;
    using namespace masala::base::utility::string;

    // Were options loaded?
    int masala_plugins_found(0);

    // Options that we will load:
    std::vector< std::string > masala_plugin_paths;

    // Options that we can load:
	struct option long_options[] {
        {"masala_plugins", required_argument, &masala_plugins_found, 1}
    };

    // Masala tracer manager:
    MasalaTracerManagerHandle tracerman( MasalaTracerManager::get_instance() );
    std::string const appname( "seating_optimization_masala_plugins::applications::applications_apps::optimize_seating" );
    tracerman->write_to_tracer( appname, "Starting optimize_seating application." );
    tracerman->write_to_tracer( appname, "Application created 15 August 2025 by Vikram K. Mulligan, Biomolecular Design Group, Center for Computational Biology, Flatiron Institute." );
    tracerman->write_to_tracer( appname, "Please write to vmulligan@flatironinstitute.org for questions.");

    // Load options:
    int option_index;
    while( !getopt_long_only( argc, argv, "", long_options, &option_index ) ){
        //std::cout << "option_index: " << option_index << std::endl; // DELETE ME
        std::string const curname(long_options[option_index].name);
        tracerman->write_to_tracer( appname, "Parsed -" + curname +  " option." );

        if( curname == "masala_plugins" ) {
            std::string optargstring(optarg);
            replace_all_instances_of_text(optargstring, ",", " ");
            std::istringstream ss( optargstring );
            while( !ss.eof() ) {
                std::string temp;
                ss >> temp;
                masala_plugin_paths.push_back( temp );
            }
            tracerman->write_to_tracer( appname, "\tGot the following Masala plugin paths: " + container_to_string( masala_plugin_paths, " " ) );
        }
    }

	return 0;

}
