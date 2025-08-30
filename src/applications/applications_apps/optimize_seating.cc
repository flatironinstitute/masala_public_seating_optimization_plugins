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
#include <base/managers/plugin_module/MasalaPluginLibraryManager.hh>
#include <base/managers/plugin_module/MasalaPluginModuleManager.hh>
#include <base/managers/engine/MasalaEngineManager.hh>
#include <base/managers/engine/MasalaEngine.hh>
#include <base/managers/engine/MasalaDataRepresentation.hh>
#include <base/managers/engine/MasalaEngineAPI.hh>
#include <base/managers/engine/MasalaDataRepresentationAPI.hh>
#include <base/utility/container/container_util.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// Masala numeric_api headers
#include <numeric_api/base_classes/optimization/cost_function_network/PluginCostFunctionNetworkOptimizer.hh>
#include <numeric_api/base_classes/optimization/cost_function_network/PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem.hh>

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

/// @brief Load all Masala plugins.
void
load_masala_plugins(
    std::vector< std::string > const & plugin_paths
) {
    using namespace masala::base::managers::plugin_module;
    MasalaPluginLibraryManagerHandle libman( MasalaPluginLibraryManager::get_instance() );
    for( auto const & plugin_path : plugin_paths ) {
        libman->load_and_register_plugin_libraries_in_subdirectories( plugin_path );
    }
}

/// @brief 

/// @brief Unload all Masala plugins.
void
unload_masala_plugins() {
    using namespace masala::base::managers::plugin_module;
    MasalaPluginLibraryManager::get_instance()->reset();
}

/// @brief Print help messages.
void
print_help_messages(
    std::map< std::string, std::string > const & help_messages,
    std::string const & appname
) {
    using masala::base::Size;
    std::ostringstream ss;
    ss << "The following options may be set:" << std::endl;

    for( std::map< std::string, std::string >::const_iterator it( help_messages.begin()); it!=help_messages.end(); ++it ) {
        ss << "-" << it->first << "\t" << it->second << std::endl;
    }

    masala::base::managers::tracer::MasalaTracerManager::get_instance()->write_to_tracer( appname, ss.str() );
}

/// @brief Load a hill-flattening Monte Carlo cost function network optimizer.
masala::base::managers::engine::MasalaEngineAPICSP
load_hill_flattening_mc_cfn_optimizer(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname
) {
	using namespace masala::base::managers::engine;
	MasalaEngineAPISP optimizer(
		MasalaEngineManager::get_instance()->create_engine_by_short_name( "HillFlatteningMonteCarloCostFunctionNetworkOptimizer", false )
	);
	CHECK_OR_THROW( optimizer != nullptr && optimizer->inner_class_name() == "HillFlatteningMonteCarloCostFunctionNetworkOptimizer",
		appname,"load_hill_flattening_mc_cfn_optimizer", "Could not load a HillFlatteningMonteCarloCostFunctionNetworkOptimizer "
		"from the Masala engine manager.  Has the Standard Masala Plugins library path been passed to the -masala_plugins commandling option?"
	);
	tracerman->write_to_tracer( appname + "load_hill_flattening_mc_cfn_optimizer", "Created a HillFlatteningMonteCarloCostFunctionNetworkOptimizer." );

	// TODO CONFIGURE HERE.

	return optimizer;
}

/// @brief Load optimizer.
masala::base::managers::engine::MasalaEngineAPICSP
load_optimizer_settings(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	std::vector< std::string > const & allowed_optimizer_names,
	std::string const & optimizer_name
) {
	if( optimizer_name == "HillFlatteningMonteCarloCostFunctionNetworkOptimizer" ) {
		return load_hill_flattening_mc_cfn_optimizer( tracerman, appname );
	} else {
		MASALA_THROW( appname, "load_optimizer_settings", "Did not recognize \"" + optimizer_name + "\" as an allowed optimizer.  "
			"Supported optimizers are: " + masala::base::utility::container::container_to_string( allowed_optimizer_names, ", " ) + "."
		);
	}
	return nullptr;
} 

/// @brief Load options.
bool
load_options(
	int const & argc,
	char * const argv[],
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	std::vector< std::string > const & allowed_optimizer_names,
	std::vector< std::string > & masala_plugin_paths,
	std::string & optimizer_name,
	int & help_indicated,
	int & masala_plugins_found,
	int & optimizer_name_specified
) {
	using namespace masala::base::utility::container;
	using namespace masala::base::utility::string;
	using namespace masala::base::managers::tracer;

	// Options that we can load:
	option long_options[] {
		{"h", no_argument, &help_indicated, 1},
		{"help", no_argument, &help_indicated, 1},
		{"masala_plugins", required_argument, &masala_plugins_found, 1},
		{"optimizer_name", required_argument, &optimizer_name_specified, 1},
	};
	std::map< std::string, std::string > const help_messages{
		{"h", "Print a help message and exit."},
		{"help", "Print a help message and exit."},
		{"masala_plugins", "The paths to the masala plugins that will be loaded, as a comma-separated list."},
		{"optimizer_name", "The name of the optimizer to use to solve the seating optimization problem.  Currently supported "
			"optimizers include: " + masala::base::utility::container::container_to_string( allowed_optimizer_names, ", " ) + "."
		}
	};

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
			tracerman->write_to_tracer( appname, "\tGot the following Masala plugin paths: " + container_to_string( masala_plugin_paths, ", " ) + "." );
		} else if( curname == "optimizer_name" ) {
			optimizer_name = std::string(optarg);
		}
	}

	// Stop with help message if help has been requeseted:
	if( help_indicated == 1 ) {
		print_help_messages( help_messages, appname );
		return false;
	}
	return true;
}

/// @brief Program entry point:
int 
main(
	int argc,
	char * argv[]
) {
    using namespace masala::base::managers::tracer;
    using namespace masala::base::managers::engine;

    // Were options loaded?
    int help_indicated(0);
    int masala_plugins_found(0);
    int optimizer_name_specified(0);

	// Allowed optimizer names:
	std::vector< std::string > const allowed_optimizer_names{
		"HillFlatteningMonteCarloCostFunctionNetworkOptimizer"
	};

    // Options that we will load:
    std::vector< std::string > masala_plugin_paths;
    std::string optimizer_name;

    // Masala tracer manager:
    MasalaTracerManagerHandle tracerman( MasalaTracerManager::get_instance() );
    std::string const appname( "seating_optimization_masala_plugins::applications::applications_apps::optimize_seating" );
    tracerman->write_to_tracer( appname, "Starting optimize_seating application." );
    tracerman->write_to_tracer( appname, "Application created 15 August 2025 by Vikram K. Mulligan, Biomolecular Design Group, Center for Computational Biology, Flatiron Institute." );
    tracerman->write_to_tracer( appname, "Please write to vmulligan@flatironinstitute.org for questions.");

    // Load options:
    if(
		!load_options(
			argc, argv, tracerman, appname,
			allowed_optimizer_names, masala_plugin_paths, optimizer_name,
			help_indicated, masala_plugins_found, optimizer_name_specified
		)
	) {
		return 0;
	}

    // Load masala plugins:
    load_masala_plugins( masala_plugin_paths );

    // Load the optimizer settings.  This fully configures the optimizer.
    CHECK_OR_THROW( optimizer_name_specified == 1, appname, "main", "An optimizer must be specified with the -optimizer_name flag." );
    MasalaEngineAPICSP optimizer_api(
		load_optimizer_settings(
			tracerman,
			appname,
			allowed_optimizer_names,
			optimizer_name
		)
	);

    // Load the problem specification:
    //load_problem_specification();
    
    // Print a summary of the setup:
   // print_setup_sumamry();

    // Solve the problem:
    //solve_problem();

    // Print the solution(s):
    //print_solutions();

    unload_masala_plugins();

	return 0;

}
