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
#include <base/managers/threads/MasalaThreadManager.hh>
#include <base/managers/engine/MasalaEngineManager.hh>
#include <base/managers/engine/MasalaDataRepresentationManager.hh>
#include <base/managers/engine/MasalaEngine.hh>
#include <base/managers/engine/MasalaDataRepresentation.hh>
#include <base/managers/engine/MasalaEngineAPI.hh>
#include <base/managers/engine/MasalaDataRepresentationAPI.hh>
#include <base/managers/disk/MasalaDiskManager.hh>
#include <base/utility/container/container_util.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>

// Masala core_api headers:
#include <core_api/auto_generated_api/registration/register_core.hh>

// Masala numeric_api headers:
#include <numeric_api/auto_generated_api/registration/register_numeric.hh>
#include <numeric_api/base_classes/optimization/cost_function_network/PluginCostFunctionNetworkOptimizer.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem_API.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationProblems_API.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationSolutions_API.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationSolution_API.hh>

// Seating problem API headers:
#include <seating_optimization_api/auto_generated_api/seating_problem/SeatingProblem_API.hh>
#include <seating_optimization_api/auto_generated_api/seating_problem/SeatingSolution_API.hh>

// STL headers:
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <getopt.h>
#include <iostream>

using masala::base::Size;
using masala::base::Real;

/// @brief Define the mapping type to use.
enum class DWaveMappingType {
	INVALID_TYPE = 0, // Keep first.
	ONE_HOT,
	DOMAIN_WALL,
	APPROXIMATE_BINARY, // Keep second-to-last.
	N_MAPPING_TYPES=APPROXIMATE_BINARY // Keep last.
};

/// @brief Load all Masala plugins.
void
load_masala_plugins(
	std::vector< std::string > const & plugin_paths
) {
	using namespace masala::base::managers::plugin_module;
	using namespace seating_optimization_masala_plugins::registration_api;

	masala::numeric_api::auto_generated_api::registration::register_numeric();
	masala::core_api::auto_generated_api::registration::register_core();

	register_library();
	MasalaPluginLibraryManagerHandle libman( MasalaPluginLibraryManager::get_instance() );
	for( auto const & plugin_path : plugin_paths ) {
		libman->load_and_register_plugin_libraries_in_subdirectories( plugin_path );
	}
}

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

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
template< typename T >
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	T const setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< T > setter(
		api_def.get_oneinput_setter_function< T >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + std::to_string(setting) + ")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
template<>
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	masala::base::managers::engine::MasalaDataRepresentationAPICSP const & setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< masala::base::managers::engine::MasalaDataRepresentationAPICSP const & > setter(
		api_def.get_oneinput_setter_function< masala::base::managers::engine::MasalaDataRepresentationAPICSP const & >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + setting->inner_class_name() + ")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
template<>
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	masala::base::managers::plugin_module::MasalaPluginAPISP const & setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< masala::base::managers::plugin_module::MasalaPluginAPISP const & > setter(
		api_def.get_oneinput_setter_function< masala::base::managers::plugin_module::MasalaPluginAPISP const & >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + setting->inner_class_name() + ")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
template<>
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	masala::base::managers::engine::MasalaEngineAPICSP const & setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< masala::base::managers::engine::MasalaEngineAPICSP const & > setter(
		api_def.get_oneinput_setter_function< masala::base::managers::engine::MasalaEngineAPICSP const & >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + setting->inner_class_name() + ")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
template<>
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	masala::base::managers::engine::MasalaEngineAPICSP setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< masala::base::managers::engine::MasalaEngineAPICSP > setter(
		api_def.get_oneinput_setter_function< masala::base::managers::engine::MasalaEngineAPICSP >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + setting->inner_class_name() + ")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
/// @details Specialization for strings.
template<>
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	std::string const & setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< std::string const & > setter(
		api_def.get_oneinput_setter_function< std::string const & >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(\"" + setting + "\")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
/// @details Specialization for const bools.
void
set_const_bool_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	bool const setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< bool const > setter(
		api_def.get_oneinput_setter_function< bool const >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + (setting ? "true" : "false" ) + ")."
	);
}

/// @brief Set the number of classical Monte Carlo steps for a classical optimizer.
/// @details Specialization for bools.
template<>
void
set_setter(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	std::string const & setter_name,
	bool setting
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;

	MasalaObjectAPISetterDefinition_OneInputCSP< bool > setter(
		api_def.get_oneinput_setter_function< bool >( setter_name ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "set_setter", "The " + api_def.api_class_name() + " did not have a "
		+ setter_name + "() function."
	);
	setter->function(setting);
	tracerman->write_to_tracer( appname + "::set_setter", "Set " + api_def.api_class_name() + "."
		+ setter_name + "(" + (setting ? "true" : "false" ) + ")."
	);
}

/// @brief Set the annealing schedule
void
configure_annealing_schedule(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::managers::plugin_module;
	using masala::base::Real;

	MasalaPluginAPISP annealsched(
		MasalaPluginModuleManager::get_instance()->create_plugin_object_instance_by_short_name(
			{ "AnnealingSchedule" }, "LogarithmicRepeatAnnealingSchedule", true
		)
	);
	CHECK_OR_THROW( annealsched != nullptr, appname, "configure_annealing_schedule", "Could not create a LogarithmicRepeatAnnealingSchedule.  "
		"Has the standard Masala plugins library been loaded?"
	);
	MasalaObjectAPIDefinitionCSP anneal_apidef( annealsched->get_api_definition_for_inner_class().lock() );
	set_setter< Real >( tracerman, appname, *anneal_apidef, "set_temperature_initial", 100.0 );
	set_setter< Real >( tracerman, appname, *anneal_apidef, "set_temperature_final", 0.1 );
	set_setter< Size >( tracerman, appname, *anneal_apidef, "set_n_repeats", 5 );

	MasalaObjectAPISetterDefinition_OneInputCSP< masala::base::managers::plugin_module::MasalaPluginAPI const & > setter(
		api_def.get_oneinput_setter_function< masala::base::managers::plugin_module::MasalaPluginAPI const & >( "set_annealing_schedule" ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "configure_annealing_schedule", "The " + api_def.api_class_name() + " did not have a "
		+ "set_annealing_schedule" + "() function."
	);
	setter->function(*annealsched);
	tracerman->write_to_tracer( appname + "::configure_annealing_schedule", "Set " + api_def.api_class_name() + "."
		+ "set_annealing_schedule" + "(" + annealsched->inner_class_name() + ")."
	);
}

/// @brief Set the hill-flattening Boltzmann temperature for the HillFlatteningMonteCarloCostFunctionNetworkOptimizer.
void
configure_hill_flattening_kbt(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::api::MasalaObjectAPIDefinition const & api_def,
	masala::base::Real const kbt
) {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::managers::plugin_module;
	using masala::base::Real;

	MasalaPluginAPISP flattening_sched(
		MasalaPluginModuleManager::get_instance()->create_plugin_object_instance_by_short_name(
			{ "AnnealingSchedule" }, "ConstantAnnealingSchedule", true
		)
	);
	CHECK_OR_THROW( flattening_sched != nullptr, appname, "configure_hill_flattening_kbt", "Could not create a ConstantAnnealingSchedule.  "
		"Has the standard Masala plugins library been loaded?"
	);
	MasalaObjectAPIDefinitionCSP anneal_apidef( flattening_sched->get_api_definition_for_inner_class().lock() );
	set_setter< Real >( tracerman, appname, *anneal_apidef, "set_temperature", kbt );

	MasalaObjectAPISetterDefinition_OneInputCSP< masala::base::managers::plugin_module::MasalaPluginAPI const & > setter(
		api_def.get_oneinput_setter_function< masala::base::managers::plugin_module::MasalaPluginAPI const & >( "set_flattening_boltzmann_temperature_schedule" ).lock()
	);
	CHECK_OR_THROW( setter != nullptr, appname, "configure_hill_flattening_kbt", "The " + api_def.api_class_name() + " did not have a "
		+ "set_flattening_boltzmann_temperature_schedule" + "() function."
	);
	setter->function(*flattening_sched);
	tracerman->write_to_tracer( appname + "::configure_hill_flattening_kbt", "Set " + api_def.api_class_name() + "."
		+ "set_flattening_boltzmann_temperature_schedule" + "(" + flattening_sched->inner_class_name() + ")."
	);
}

/// @brief Load a Monte Carlo cost function network optimizer or a hill-flattening Monte Carlo cost function network optimizer.
masala::base::managers::engine::MasalaEngineAPICSP
load_mc_cfn_optimizer(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::Size const classical_mc_steps,
	masala::base::Size const classical_attempts_per_problem,
	masala::base::Size const solutions_to_store_per_problem,
	masala::base::Real const flattening_boltzmann_temperature,
	bool const do_greedy_refinement,
	bool const load_hill_flattening_version
) {
	using namespace masala::base::managers::engine;
	using namespace masala::base::api;
	using masala::base::Size;
	using masala::base::Real;

	std::string const optname(
		load_hill_flattening_version ?
		"HillFlatteningMonteCarloCostFunctionNetworkOptimizer" :
		"MonteCarloCostFunctionNetworkOptimizer"
	);

	MasalaEngineAPISP optimizer(
		MasalaEngineManager::get_instance()->create_engine_by_short_name( optname, false )
	);
	CHECK_OR_THROW( optimizer != nullptr && optimizer->inner_class_name() == optname,
		appname,"load_mc_cfn_optimizer", "Could not load a " + optname +
		"from the Masala engine manager.  Has the Standard Masala Plugins library path been passed to the -masala_plugins commandling option?"
	);
	tracerman->write_to_tracer( appname + "::load_mc_cfn_optimizer", "Created a " + optimizer->inner_class_name() + "." );

	// TODO CONFIGURE HERE.
	MasalaObjectAPIDefinitionCSP api_def( optimizer->get_api_definition_for_inner_class().lock() );
	CHECK_OR_THROW( api_def != nullptr, appname, "load_mc_cfn_optimizer", "Could not get an API definition for the " + optimizer->inner_class_name() + " optimizer." );
	set_setter<Size>( tracerman, appname, *api_def, "set_annealing_steps_per_attempt", classical_mc_steps );
	set_setter<Size>( tracerman, appname, *api_def, "set_cpu_threads_to_request", 0 );
	set_setter<Size>( tracerman, appname, *api_def, "set_attempts_per_problem", classical_attempts_per_problem );
	set_setter<Size>( tracerman, appname, *api_def, "set_n_solutions_to_store_per_problem", solutions_to_store_per_problem );
	set_setter<std::string const &>( tracerman, appname, *api_def, "set_solution_storage_mode", "check_on_acceptance" );
	set_setter<std::string const &>( tracerman, appname, *api_def, "set_greedy_refinement_mode", "refine_top" );
	set_const_bool_setter( tracerman, appname, *api_def, "set_do_greedy_refinement", do_greedy_refinement );
	if( load_hill_flattening_version ) {
		configure_hill_flattening_kbt( tracerman, appname, *api_def, flattening_boltzmann_temperature );
	}

	// Set annealing schedule:
	configure_annealing_schedule( tracerman, appname, *api_def );

	return optimizer;
}

/// @brief Load a D-Wave optimizer.
masala::base::managers::engine::MasalaEngineAPICSP
load_dwave_cfn_optimizer(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	masala::base::Size const dwave_samples,
	masala::base::Size const solutions_to_store_per_problem,
	masala::base::Real const annealing_time,
	bool const do_greedy,
	bool const use_layout_embedding,
	std::string const & dwave_solver_name,
	masala::base::Real const dwave_onenode_penalty_cap,
	masala::base::Real const dwave_twonode_penalty_cap,
	DWaveMappingType const mapping_type
) {
	using namespace masala::base::managers::engine;
	using namespace masala::base::managers::plugin_module;
	using namespace masala::base::api;
	using masala::base::Size;
	using masala::base::Real;

	MasalaEngineAPISP optimizer(
		MasalaEngineManager::get_instance()->create_engine_by_short_name( "DWaveQuantumQUBOProblemOptimizer", false )
	);
	CHECK_OR_THROW( optimizer != nullptr && optimizer->inner_class_name() == "DWaveQuantumQUBOProblemOptimizer",
		appname,"load_dwave_cfn_optimizer", "Could not load a DWaveQuantumQUBOProblemOptimizer "
		"from the Masala engine manager.  Has the Quantum Computing Masala Plugins library path "
		"been passed to the -masala_plugins commandling option?"
	);
	tracerman->write_to_tracer( appname + "::load_mc_cfn_optimizer", "Created a " + optimizer->inner_class_name() + "." );
	MasalaObjectAPIDefinitionCSP opt_api_def( optimizer->get_api_definition_for_inner_class().lock() );
	CHECK_OR_THROW( opt_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + optimizer->inner_class_name() + " optimizer." );

	if( mapping_type == DWaveMappingType::APPROXIMATE_BINARY ) {
		// Set the preferred data representation:
		MasalaDataRepresentationAPISP template_dr(
			MasalaDataRepresentationManager::get_instance()->create_data_representation_by_short_name( "ApproximateBinaryQUBOProblem", false )
		);
		CHECK_OR_THROW( template_dr != nullptr && template_dr->inner_class_name() == "ApproximateBinaryQUBOProblem",
			appname,"load_dwave_cfn_optimizer", "Could not create an ApproximateBinaryQUBOProblem "
			"from the Masala data representation manager.  Has the Quantum Computing Masala Plugins "
			"library path been passed to the -masala_plugins commandling option?"
		);
		tracerman->write_to_tracer( appname + "::load_mc_cfn_optimizer", "Created a " + template_dr->inner_class_name() + " template data representation." );

		// Configure:
		MasalaObjectAPIDefinitionCSP abqp_api_def( template_dr->get_api_definition_for_inner_class().lock() );
		CHECK_OR_THROW( abqp_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + template_dr->inner_class_name() + " optimizer." );

		set_setter<Real>( tracerman, appname, *abqp_api_def, "set_onenode_penalty_cap", dwave_onenode_penalty_cap );
		set_setter<Real>( tracerman, appname, *abqp_api_def, "set_twonode_penalty_cap", dwave_twonode_penalty_cap );
		set_const_bool_setter( tracerman, appname, *abqp_api_def, "set_optimize_onenode_penalties", true );
		set_const_bool_setter( tracerman, appname, *abqp_api_def, "set_optimize_twonode_penalties", true );
		set_setter<Real const>( tracerman, appname, *abqp_api_def, "set_local_optimizer_kbt", 12.0 );
		set_setter<Real>( tracerman, appname, *abqp_api_def, "set_weighted_linear_algebra_kbt", 12.0 );
		set_setter<bool>( tracerman, appname, *abqp_api_def, "set_exclude_duplicate_bitstrings_from_initial_linear_algebra", true );
		set_setter<bool>( tracerman, appname, *abqp_api_def, "set_use_weighted_linear_algebra", true );
		set_setter<std::string const &>( tracerman, appname, *abqp_api_def, "set_linear_algebra_approach", "col_pivoting_householder_qr_decomp" );
		set_setter<Size>( tracerman, appname, *abqp_api_def, "set_cpu_threads_to_request", 0 );
		set_const_bool_setter( tracerman, appname, *abqp_api_def, "set_compute_qubit_effective_fields", true );

		// Configure minimization engine:
		{
			MasalaEngineAPISP minimizer(
				MasalaEngineManager::get_instance()->create_engine_by_short_name( "BFGSFunctionOptimizer", false )
			);
			CHECK_OR_THROW( minimizer != nullptr && minimizer->inner_class_name() == "BFGSFunctionOptimizer",
				appname,"load_dwave_cfn_optimizer", "Could not load a BFGSFunctionOptimizer "
				"from the Masala engine manager.  Has the Standard Masala Plugins library path "
				"been passed to the -masala_plugins commandling option?"
			);
			MasalaObjectAPIDefinitionCSP minimizer_api_def( minimizer->get_api_definition_for_inner_class().lock() );
			CHECK_OR_THROW( minimizer_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + minimizer->inner_class_name() + " optimizer." );
			set_setter<Size>( tracerman, appname, *minimizer_api_def, "set_max_iterations", 1000 );
			set_setter<Size>( tracerman, appname, *minimizer_api_def, "set_threads_to_request", 0 );
			set_setter<bool>( tracerman, appname, *minimizer_api_def, "set_throw_if_iterations_exceeded", false );

			{
				// Configure line minimizer:
				MasalaEngineAPISP lineminimizer(
					MasalaEngineManager::get_instance()->create_engine_by_short_name( "BrentAlgorithmLineOptimizer", false )
				);
				CHECK_OR_THROW( lineminimizer != nullptr && lineminimizer->inner_class_name() == "BrentAlgorithmLineOptimizer",
					appname,"load_dwave_cfn_optimizer", "Could not load a BrentAlgorithmLineOptimizer "
					"from the Masala engine manager.  Has the Standard Masala Plugins library path "
					"been passed to the -masala_plugins commandling option?"
				);
				MasalaObjectAPIDefinitionCSP lineminimizer_api_def( lineminimizer->get_api_definition_for_inner_class().lock() );
				CHECK_OR_THROW( lineminimizer_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + lineminimizer->inner_class_name() + " optimizer." );
				set_setter<Real>( tracerman, appname, *lineminimizer_api_def, "set_initial_stepsize", 0.05 );
				set_setter<Size>( tracerman, appname, *lineminimizer_api_def, "set_max_iters", 20 );
				set_setter<bool>( tracerman, appname, *lineminimizer_api_def, "set_throw_if_iterations_exceeded", false );

				set_setter<MasalaEngineAPICSP>( tracerman, appname, *minimizer_api_def, "set_line_optimizer", lineminimizer );
			}

			set_setter<MasalaEngineAPICSP const &>( tracerman, appname, *abqp_api_def, "set_realvalued_local_function_optimizer", minimizer );
		}


		// Configure choice order:
		{
			MasalaPluginAPISP choiceorder(
				MasalaPluginModuleManager::get_instance()->create_plugin_object_instance_by_short_name( { "ChoiceOrder" }, "BoltzmannAverageEnergyChoiceOrder", true )
			);
			CHECK_OR_THROW( choiceorder != nullptr && choiceorder->inner_class_name() == "BoltzmannAverageEnergyChoiceOrder",
				appname,"load_dwave_cfn_optimizer", "Could not load a BoltzmannAverageEnergyChoiceOrder "
				"from the Masala plugin manager.  Has the Quantum Computing Masala Plugins library path "
				"been passed to the -masala_plugins commandling option?"
			);
			MasalaObjectAPIDefinitionCSP choiceorder_api_def( choiceorder->get_api_definition_for_inner_class().lock() );
			CHECK_OR_THROW( choiceorder_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + choiceorder->inner_class_name() + " class." );
			set_setter<std::string const &>( tracerman, appname, *choiceorder_api_def, "set_bitstring_assignment_order", "normal_with_lin_indep_priority" );
			set_setter<Real>( tracerman, appname, *choiceorder_api_def, "set_kbt", 12.0 );
			set_setter<Size>( tracerman, appname, *choiceorder_api_def, "set_n_threads", 0 );

			set_setter<MasalaPluginAPISP const &>( tracerman, appname, *abqp_api_def, "set_choice_order", choiceorder );
		}

		set_setter<MasalaDataRepresentationAPICSP const &>( tracerman, appname, *opt_api_def, "set_template_preferred_cfn_data_representation", template_dr );
	} else if( mapping_type == DWaveMappingType::DOMAIN_WALL ) {
		// Set the preferred data representation:
		MasalaDataRepresentationAPISP template_dr(
			MasalaDataRepresentationManager::get_instance()->create_data_representation_by_short_name( "DomainWallQUBOProblem", false )
		);
		CHECK_OR_THROW( template_dr != nullptr && template_dr->inner_class_name() == "DomainWallQUBOProblem",
			appname,"load_dwave_cfn_optimizer", "Could not create an DomainWallQUBOProblem "
			"from the Masala data representation manager.  Has the Quantum Computing Masala Plugins "
			"library path been passed to the -masala_plugins commandling option?"
		);
		tracerman->write_to_tracer( appname + "::load_mc_cfn_optimizer", "Created a " + template_dr->inner_class_name() + " template data representation." );

		// Configure:
		MasalaObjectAPIDefinitionCSP dwqp_api_def( template_dr->get_api_definition_for_inner_class().lock() );
		CHECK_OR_THROW( dwqp_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + template_dr->inner_class_name() + " optimizer." );

		set_setter<Real>( tracerman, appname, *dwqp_api_def, "set_onenode_penalty_cap", dwave_onenode_penalty_cap );
		set_setter<Real>( tracerman, appname, *dwqp_api_def, "set_twonode_penalty_cap", dwave_twonode_penalty_cap );

		set_setter<MasalaDataRepresentationAPICSP const &>( tracerman, appname, *opt_api_def, "set_template_preferred_cfn_data_representation", template_dr );
	} else if( mapping_type == DWaveMappingType::ONE_HOT ) {
		// Set the preferred data representation:
		MasalaDataRepresentationAPISP template_dr(
			MasalaDataRepresentationManager::get_instance()->create_data_representation_by_short_name( "OneHotQUBOProblem", false )
		);
		CHECK_OR_THROW( template_dr != nullptr && template_dr->inner_class_name() == "OneHotQUBOProblem",
			appname,"load_dwave_cfn_optimizer", "Could not create an OneHotQUBOProblem "
			"from the Masala data representation manager.  Has the Quantum Computing Masala Plugins "
			"library path been passed to the -masala_plugins commandling option?"
		);
		tracerman->write_to_tracer( appname + "::load_mc_cfn_optimizer", "Created a " + template_dr->inner_class_name() + " template data representation." );

		// Configure:
		MasalaObjectAPIDefinitionCSP ohqp_api_def( template_dr->get_api_definition_for_inner_class().lock() );
		CHECK_OR_THROW( ohqp_api_def != nullptr, appname, "load_dwave_cfn_optimizer", "Could not get an API definition for the " + template_dr->inner_class_name() + " optimizer." );

		set_setter<Real>( tracerman, appname, *ohqp_api_def, "set_onenode_penalty_cap", dwave_onenode_penalty_cap );
		set_setter<Real>( tracerman, appname, *ohqp_api_def, "set_twonode_penalty_cap", dwave_twonode_penalty_cap );

		set_setter<MasalaDataRepresentationAPICSP const &>( tracerman, appname, *opt_api_def, "set_template_preferred_cfn_data_representation", template_dr );
	}

	set_const_bool_setter( tracerman, appname, *opt_api_def, "set_greedy", do_greedy );
	set_const_bool_setter( tracerman, appname, *opt_api_def, "set_inhomog_trans_field", true );
	set_setter<bool>( tracerman, appname, *opt_api_def, "set_remove_output_files", false );
	set_setter<Real const>( tracerman, appname, *opt_api_def, "set_annealing_time", annealing_time );
	set_setter<Size>( tracerman, appname, *opt_api_def, "set_n_solutions_to_store_per_problem", solutions_to_store_per_problem );
	set_setter<Size const>( tracerman, appname, *opt_api_def, "set_num_reads", dwave_samples );
	set_setter<std::string const &>( tracerman, appname, *opt_api_def, "set_output_directory", "./test_scratch/" );
	set_setter<std::string const &>( tracerman, appname, *opt_api_def, "set_solver", dwave_solver_name );
	set_setter<Size const>( tracerman, appname, *opt_api_def, "set_minorminer_chainlength_patience", 32 );
	set_setter<Size const>( tracerman, appname, *opt_api_def, "set_minorminer_max_no_improvement", 128 );
	set_setter<Size const>( tracerman, appname, *opt_api_def, "set_minorminer_max_no_improvement", 16 );
	set_setter<Size const>( tracerman, appname, *opt_api_def, "set_minorminer_tries", 256 );
	set_setter<std::string const &>( tracerman, appname, *opt_api_def, "set_embedding_type", (use_layout_embedding ? "layout" : "minor_miner") );

	return optimizer;
}

/// @brief Load optimizer.
masala::base::managers::engine::MasalaEngineAPICSP
load_optimizer_settings(
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	std::string const & appname,
	std::vector< std::string > const & allowed_optimizer_names,
	std::string const & optimizer_name,
	int const classical_mc_steps_specified,
	masala::base::Size const classical_mc_steps,
	int const classical_attempts_per_problem_specified,
	masala::base::Size const classical_attempts_per_problem,
	int const /*solutions_to_store_per_problem_specified*/, // Used for all optimizers, so no checks here.
	masala::base::Size const solutions_to_store_per_problem,
	int const flattening_boltzmann_temperature_specified,
	masala::base::Real const flattening_boltzmann_temperature,
	int const /*do_greedy_specified*/, // Used for all optimizers, so no checks here.
	bool const do_greedy,
	int const dwave_samples_specified,
	masala::base::Size const dwave_samples,
	int const dwave_annealing_time_specified,
	masala::base::Real const dwave_annealing_time,
	int const dwave_use_layout_embedding_specified,
	bool const dwave_use_layout_embedding,
	int const dwave_solver_name_specified,
	std::string const & dwave_solver_name,
	int const dwave_onenode_penalty_cap_specified,
	masala::base::Real const dwave_onenode_penalty_cap,
	int const dwave_twonode_penalty_cap_specified,
	masala::base::Real const dwave_twonode_penalty_cap
) {
	// Initial checks:
	if( !( optimizer_name == "HillFlatteningMonteCarloCostFunctionNetworkOptimizer" || optimizer_name == "MonteCarloCostFunctionNetworkOptimizer" ) ) {
		CHECK_OR_THROW( !classical_mc_steps_specified,
			appname, "load_optimizer_settings", "Classical Monte Carlo steps were specified, but the selected optimizer does not perform classical Monte Carlo."
		);
		CHECK_OR_THROW( !classical_attempts_per_problem_specified,
			appname, "load_optimizer_settings", "Classical attempts per problem were specified, but the selected optimizer does not perform classical Monte Carlo."
		);
	}
	if( !( optimizer_name == "HillFlatteningMonteCarloCostFunctionNetworkOptimizer" ) ) {
		CHECK_OR_THROW( !flattening_boltzmann_temperature_specified,
			appname, "load_optimizer_settings", "A flattening Boltzmann temperature was specified, but the optimizer is not the HillFlatteningMonteCarloCostFunctionNetworkOptimizer."
		);
	}
	if( !(optimizer_name == "DWaveQuantumQUBOProblemOptimizer") ) {
		CHECK_OR_THROW( !dwave_samples_specified, appname, "load_optimizer_settings", "A D-Wave sample count was specified, but the optimizer is not the DWaveQuantumQUBOProblemOptimizer." );
		CHECK_OR_THROW( !dwave_annealing_time_specified, appname, "load_optimizer_settings", "A D-Wave annealing time was specified, but the optimizer is not the DWaveQuantumQUBOProblemOptimizer." );
		CHECK_OR_THROW( !dwave_use_layout_embedding_specified, appname, "load_optimizer_settings", "A D-Wave embedding type was specified, but the optimizer is not the DWaveQuantumQUBOProblemOptimizer." );
		CHECK_OR_THROW( !dwave_solver_name_specified, appname, "load_optimizer_settings", "A D-Wave solver name was specified, but the optimizer is not the DWaveQuantumQUBOProblemOptimizer." );
		CHECK_OR_THROW( !dwave_onenode_penalty_cap_specified, appname, "load_optimizer_settings", "A D-Wave one-node penalty cap was specified, but the optimizer is not the DWaveQuantumQUBOProblemOptimizer." );
		CHECK_OR_THROW( !dwave_twonode_penalty_cap_specified, appname, "load_optimizer_settings", "A D-Wave two-node penalty cap was specified, but the optimizer is not the DWaveQuantumQUBOProblemOptimizer." );
	}

	if( optimizer_name == "HillFlatteningMonteCarloCostFunctionNetworkOptimizer" ) {
		return load_mc_cfn_optimizer( tracerman, appname, classical_mc_steps, classical_attempts_per_problem,
			solutions_to_store_per_problem, flattening_boltzmann_temperature, do_greedy, true
		);
	} else if( optimizer_name == "MonteCarloCostFunctionNetworkOptimizer" ) {
		return load_mc_cfn_optimizer( tracerman, appname, classical_mc_steps, classical_attempts_per_problem,
			solutions_to_store_per_problem, flattening_boltzmann_temperature, do_greedy, false
		);
	} else if( optimizer_name == "DWaveQuantumQUBOProblemOptimizer" ) {
		CHECK_OR_THROW( dwave_solver_name_specified, appname, "load_optimizer_settings", "A D-Wave solver must be specified (-dwave_solver_name commandline option) to use the DWaveQuantumQUBOProblemOptimizer." );
		return load_dwave_cfn_optimizer( tracerman, appname, dwave_samples, solutions_to_store_per_problem, dwave_annealing_time, do_greedy, dwave_use_layout_embedding, dwave_solver_name, dwave_onenode_penalty_cap, dwave_twonode_penalty_cap );
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
	masala::base::Size & classical_mc_steps,
	masala::base::Size & total_threads,
	masala::base::Size & classical_attempts_per_problem,
	masala::base::Size & solutions_to_store_per_problem,
	masala::base::Real & flattening_boltzmann_temperature,
	bool & do_greedy,
	std::string & probfile_name,
	int & help_indicated,
	int & masala_plugins_found,
	int & optimizer_name_specified,
	int & classical_mc_steps_specified,
	int & total_threads_specified,
	int & classical_attempts_per_problem_specified,
	int & solutions_to_store_per_problem_specified,
	int & flattening_boltzmann_temperature_specified,
	int & do_greedy_specified,
	int & problem_file_specified,
	int & dwave_samples_specified,
	masala::base::Size & dwave_samples,
	int & dwave_annealing_time_specified,
	masala::base::Real & dwave_annealing_time,
	int & dwave_use_layout_embedding_specified,
	bool & dwave_use_layout_embedding,
	int & dwave_solver_name_specified,
	std::string & dwave_solver_name,
	int & dwave_onenode_penalty_cap_specified,
	masala::base::Real & dwave_onenode_penalty_cap,
	int & dwave_twonode_penalty_cap_specified,
	masala::base::Real & dwave_twonode_penalty_cap
) {
	using namespace masala::base::utility::container;
	using namespace masala::base::utility::string;
	using namespace masala::base::managers::tracer;
	using masala::base::Size;
	using masala::base::Real;

	// Options that we can load:
	option long_options[] {
		{"h", no_argument, &help_indicated, 1},
		{"help", no_argument, &help_indicated, 1},
		{"masala_plugins", required_argument, &masala_plugins_found, 1},
		{"optimizer_name", required_argument, &optimizer_name_specified, 1},
		{"classical_mc_steps", required_argument, &classical_mc_steps_specified, 1},
		{"total_threads", required_argument, &total_threads_specified, 1},
		{"classical_attempts_per_problem", required_argument, &classical_attempts_per_problem_specified, 1},
		{"solutions_to_store_per_problem", required_argument, &solutions_to_store_per_problem_specified, 1},
		{"flattening_boltzmann_temperature", required_argument, &flattening_boltzmann_temperature_specified, 1},
		{"do_greedy", required_argument, &do_greedy_specified, 1},
		{"problem_file", required_argument, &problem_file_specified, 1},
		{"dwave_samples", required_argument, &dwave_samples_specified, 1},
		{"dwave_annealing_time", required_argument, &dwave_annealing_time_specified, 1},
		{"dwave_use_layout_embedding", required_argument, &dwave_use_layout_embedding_specified, 1},
		{"dwave_solver_name", required_argument, &dwave_solver_name_specified, 1},
		{"dwave_onenode_penalty_cap", required_argument, &dwave_onenode_penalty_cap_specified, 1},
		{"dwave_twonode_penalty_cap", required_argument, &dwave_twonode_penalty_cap_specified, 1}
	};
	std::map< std::string, std::string > const help_messages{
		{"h", "Print a help message and exit."},
		{"help", "Print a help message and exit."},
		{"masala_plugins", "The paths to the masala plugins that will be loaded, as a comma-separated list."},
		{"optimizer_name", "The name of the optimizer to use to solve the seating optimization problem.  Currently supported "
			"optimizers include: " + masala::base::utility::container::container_to_string( allowed_optimizer_names, ", " ) + "."
		},
		{"classical_mc_steps", "The number of Monte Carlo steps per attempt that classical optimizers, such as the "
			"MonteCarloCostFunctionNetworkOptimizer or the HillFlatteningMonteCarloCostFunctionNetworkOptimizer, will make.  "
			"Defaults to 1,000,000 if not specified."
		},
		{"total_threads", "The number of threads to launch.  Defaults to 1.  Zero means to launch one thread per CPU core on the node."},
		{"classical_attempts_per_problem", "The number of attempts (Monte Carlo trajectories) that classical optimizers, such as the "
			"MonteCarloCostFunctionNetworkOptimizer or the HillFlatteningMonteCarloCostFunctionNetworkOptimizer, will run.  "
			"Defaults to 1 if not specified."
		},
		{"solutions_to_store_per_problem", "The maximum number of solutions to return, for any optimizer.  Defaults to 1." },
		{"flattening_boltzmann_temperature", "The Boltzmann temperature to use for flattening the solution score landscape, if the "
			"HillFlatteningMonteCarloCostFunctionNetworkOptimizer is used.  Defaults to 10.0."
		},
		{"do_greedy", "Should solutions be greedily refined?  This is an option for all optimizers; must be TRUE or "
			"FALSE.  Defaults to TRUE."
		},
		{"problem_file", "The name (with absolute or relative path) of the file defining the seating optimization problem to "
			"solve.  Required input."
		},
		{"dwave_samples", "The number of samples for the D-Wave to perform.  Defaults to 1000." },
		{"dwave_annealing_time", "The D-wave annealing time, in microseconds.  Defaults to 20.0" },
		{"dwave_use_layout_embedding", "If true, we use Layout embedding; if false, we use MinorMiner embedding.  Defaults to true." },
		{"dwave_solver_name", "The name of the solver.  Required input if the D-Wave is used." },
		{"dwave_onenode_penalty_cap", "The cap on the one-node penalty values.  Only used to limit dynamic range if the D-Wave is used.  Defaults to 100.0." },
		{"dwave_twonode_penalty_cap", "The cap on the two-node penalty values.  Only used to limit dynamic range if the D-Wave is used.  Defaults to 100.0." }
	};

	int option_index;
	while( !getopt_long_only( argc, argv, "", long_options, &option_index ) ){
		//std::cout << "option_index: " << option_index << std::endl; // DELETE ME
		std::string const curname(long_options[option_index].name);
		tracerman->write_to_tracer( appname, "Parsed -" + curname +  " option." );

		if( curname == "h" || curname == "help" ) {
			continue;
		}else if( curname == "masala_plugins" ) {
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
		} else if( curname == "classical_mc_steps" ) {
			std::istringstream ss( optarg );
			long signed int temp;
			ss >> temp;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as an integer." );
			CHECK_OR_THROW( temp > 0, appname, "load_options", "The number of classical Monte Carlo steps must be a positive integer." );
			classical_mc_steps = static_cast< Size >(temp);
		} else if( curname == "total_threads" ) {
			std::istringstream ss( optarg );
			long signed int temp;
			ss >> temp;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as an integer." );
			CHECK_OR_THROW( temp >= 0, appname, "load_options", "The number of total threads must be a non-negative integer.  Zero means to launch one thread per CPU core." );
			total_threads = static_cast< Size >(temp);
		} else if( curname == "classical_attempts_per_problem" ) {
			std::istringstream ss( optarg );
			long signed int temp;
			ss >> temp;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as an integer." );
			CHECK_OR_THROW( temp > 0, appname, "load_options", "The number of classical attempts per problem must be a positive integer." );
			classical_attempts_per_problem = static_cast< Size >(temp);
		} else if( curname == "solutions_to_store_per_problem" ) {
			std::istringstream ss( optarg );
			long signed int temp;
			ss >> temp;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as an integer." );
			CHECK_OR_THROW( temp > 0, appname, "load_options", "The number solutions to store per problem must be a positive integer." );
			solutions_to_store_per_problem = static_cast< Size >(temp);
		} else if( curname == "flattening_boltzmann_temperature" ) {
			std::istringstream ss( optarg );
			ss >> flattening_boltzmann_temperature;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as a floating-point number." );
			CHECK_OR_THROW( flattening_boltzmann_temperature >= 0.0, appname, "load_options", "The Boltzmann temperature for flattening the solution score landscape must be non-negative." );
		} else if( curname == "do_greedy" ) {
			std::string const greedystring( optarg );
			if( greedystring == "TRUE" ) {
				do_greedy = true;
			} else if( greedystring == "FALSE" ) {
				do_greedy = false;
			} else {
				MASALA_THROW( appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as a Boolean.  Must be either TRUE or FALSE." );
			}
		} else if( curname == "problem_file" ) {
			probfile_name = std::string( optarg );
			CHECK_OR_THROW( !probfile_name.empty(), appname, "load_options", "The problem file name must not be empty." ); 
		} else if( curname == "dwave_samples" ) {
			std::istringstream ss( optarg );
			long signed int temp;
			ss >> temp;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as an integer." );
			CHECK_OR_THROW( temp > 0, appname, "load_options", "The number D-wave samples must be a positive integer." );
			dwave_samples = static_cast< Size >(temp);
		} else if( curname == "dwave_annealing_time" ) {
			std::istringstream ss( optarg );
			Real temp;
			ss >> temp;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as a floating-point number." );
			CHECK_OR_THROW( temp > 0, appname, "load_options", "The D-Wave annealing time must be positive." );
			dwave_annealing_time = temp;
		} else if( curname == "dwave_use_layout_embedding" ) {
			std::string const layoutstring( optarg );
			if( layoutstring == "TRUE" ) {
				dwave_use_layout_embedding = true;
			} else if( layoutstring == "FALSE" ) {
				dwave_use_layout_embedding = false;
			} else {
				MASALA_THROW( appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as a Boolean.  Must be either TRUE or FALSE." );
			}
		} else if( curname == "dwave_solver_name" ) {
			dwave_solver_name = std::string( optarg );
			CHECK_OR_THROW( !dwave_solver_name.empty(), appname, "load_options", "The D-wave solver name must not be empty." ); 
		} else if( curname == "dwave_onenode_penalty_cap" ) {
			std::istringstream ss( optarg );
			ss >> dwave_onenode_penalty_cap;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as a floating-point number." );
			CHECK_OR_THROW( dwave_onenode_penalty_cap > 0, appname, "load_options", "The D-Wave one-node penalty cap must be positive." );
		} else if( curname == "dwave_twonode_penalty_cap" ) {
			std::istringstream ss( optarg );
			ss >> dwave_twonode_penalty_cap;
			CHECK_OR_THROW( ss.eof() && !(ss.bad() || ss.fail() ), appname, "load_options", "Could not parse \"" + std::string(optarg) + "\" as a floating-point number." );
			CHECK_OR_THROW( dwave_twonode_penalty_cap > 0, appname, "load_options", "The D-Wave two-node penalty cap must be positive." );
		} else {
			MASALA_THROW( appname, "load_options", "Unknown command-line option \"" + curname + "\"." );
		}
	}

	// Stop with help message if help has been requeseted:
	if( help_indicated == 1 ) {
		print_help_messages( help_messages, appname );
		return false;
	}
	return true;
}

/// @brief Load a seating problem from a file:
seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingProblem_APISP
load_problem_specification(
	std::string const & appname,
	std::string const & probfile_name
) {
	using namespace seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem;
	using namespace masala::base::managers::disk;
	SeatingProblem_APISP seating_problem( masala::make_shared< SeatingProblem_API >() );
	CHECK_OR_THROW( seating_problem != nullptr, appname, "load_problem_specification", "Failed to create seating problem definition." );
	std::vector< std::string > const filelines( MasalaDiskManager::get_instance()->read_ascii_file_to_string_vector( probfile_name ) );
	seating_problem->configure_from_problem_definition_file_lines( filelines );
	seating_problem->finalize();
	return seating_problem;
}

/// @brief Run the CFN problem and produce a solution.
std::vector< seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingSolution_APICSP >
solve_problem(
	std::string const & appname,
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	masala::base::managers::engine::MasalaEngineAPI const & optimizer_api,
	seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingProblem_APICSP seating_problem,
	masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationProblems_API const & problems,
	std::vector< std::map< masala::base::Size, masala::base::Size > > const & guest_choice_to_seat_index
) {
	using masala::base::Size;
	using namespace masala::numeric_api::base_classes::optimization::cost_function_network;
	using namespace masala::numeric_api::auto_generated_api::optimization::cost_function_network;
	using namespace seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem;

	CHECK_OR_THROW( problems.n_problems() == 1, appname, "solve_problem", "Expected 1 problem in the problems container, but got " + std::to_string(problems.n_problems()) + "." );

	PluginCostFunctionNetworkOptimizerCSP optimizer(
		std::dynamic_pointer_cast< PluginCostFunctionNetworkOptimizer const >( optimizer_api.get_inner_engine_object_const() )
	);
	CHECK_OR_THROW( optimizer != nullptr, appname, "solve_problem", "Could not interpret an object of type \"" + optimizer_api.inner_class_name() + "\" as a CFN optimizer." );

	std::vector< masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationSolutions_APICSP > solutions_vec(
		optimizer->run_cost_function_network_optimizer( problems )
	);
	CHECK_OR_THROW( solutions_vec.size() == 1, appname, "solve_problem", "Expected 1 solution set, but got " + std::to_string( solutions_vec.size() ) + "." );
	masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationSolutions_API const & solutions( *solutions_vec[0] );
	CHECK_OR_THROW( solutions.n_solutions() > 0, appname, "solve_problem", "Expected at least one solution." );

	// Make a container for the solutions:
	std::vector< SeatingSolution_APICSP > seating_solutions;
	seating_solutions.reserve(solutions.n_solutions());

	tracerman->write_to_tracer(appname, "INDEX\tTIMES_SEEN\tSOLUTION\tACTUAL_SCORE\tDR_APPROX_SCORE\tSOLVER_APPROX_SCORE\tVALID");
	for( Size i(0); i<solutions.n_solutions(); ++i ) {
		CostFunctionNetworkOptimizationSolution_APICSP cursolution( std::dynamic_pointer_cast< CostFunctionNetworkOptimizationSolution_API const >(solutions.solution(i)) );
		CHECK_OR_THROW( cursolution != nullptr, appname, "solve_problem", "Solution " + std::to_string(i) + " was not a CostFunctionNetworkOptimizationSolution." );
		tracerman->write_to_tracer( appname, std::to_string(i) + "\t" + std::to_string(cursolution->n_times_solution_was_produced()) + "\t[" +
			masala::base::utility::container::container_to_string(
				 cursolution->solution_at_variable_positions(), ","
			) + "]\t" + std::to_string(cursolution->solution_score()) + "\t" + std::to_string(cursolution->solution_score_data_representation_approximation())
			+ "\t" + std::to_string(cursolution->solution_score_solver_approximation()) + "\t" + ( cursolution->solution_is_valid() ? "TRUE" : "FALSE" )
		);
		if( cursolution->solution_is_valid() ) {
			SeatingSolution_APISP seating_solution( seating_problem->seating_solution_from_cfn_solution( *cursolution, guest_choice_to_seat_index ) );
			seating_solution->set_problem( seating_problem );
			seating_solution->finalize();
			seating_solutions.push_back( seating_solution );
		}
	}
	seating_solutions.shrink_to_fit();

	return seating_solutions;
}

/// @brief Set up the CFN problem and package it in a problems container.
masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationProblems_APISP
set_up_cfn_problem(
	std::string const & appname,
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	masala::base::managers::engine::MasalaEngineAPI const & optimizer_api,
	seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingProblem_API const & seating_problem,
	std::vector< std::vector< bool > > & allowed_seats_by_guest,
	std::vector< std::map< masala::base::Size, masala::base::Size > > & guest_choice_to_seat_index,
	std::vector< std::map< masala::base::Size, masala::base::Size > > & seat_index_to_guest_choice
) {
	using masala::base::Size;
	using namespace masala::numeric_api::base_classes::optimization::cost_function_network;
	using namespace masala::numeric_api::auto_generated_api::optimization::cost_function_network;

	PluginCostFunctionNetworkOptimizerCSP optimizer(
		std::dynamic_pointer_cast< PluginCostFunctionNetworkOptimizer const >( optimizer_api.get_inner_engine_object_const() )
	);
	CHECK_OR_THROW( optimizer != nullptr, appname, "set_up_cfn_problem", "Could not interpret an object of type \"" + optimizer_api.inner_class_name() + "\" as a CFN optimizer." );

	masala::base::managers::engine::MasalaDataRepresentationAPISP problem_uncast( optimizer->get_template_preferred_cfn_data_representation_copy() );
	CHECK_OR_THROW( problem_uncast != nullptr, appname, "set_up_cfn_problem", "Could not get a template-preferred data representation from the solver of type \"" + optimizer_api.inner_class_name() + "\"." );
	CostFunctionNetworkOptimizationProblem_APISP problem( std::dynamic_pointer_cast< CostFunctionNetworkOptimizationProblem_API >( problem_uncast ) );
	CHECK_OR_THROW( problem != nullptr, appname, "set_up_cfn_problem", "Could not interpret an object of type \"" + problem_uncast->inner_class_name() + "\" as a CFN optimization problem." );
	tracerman->write_to_tracer( appname, "Created a problem container of class \"" + problem->inner_class_name() + "\"." );

	// Configure and finalize here:
	seating_problem.set_up_cfn_problem( *problem, allowed_seats_by_guest, guest_choice_to_seat_index, seat_index_to_guest_choice );

	// Package in a problems container:
	CostFunctionNetworkOptimizationProblems_APISP problems( masala::make_shared< CostFunctionNetworkOptimizationProblems_API >() );
	problems->add_optimization_problem(problem);

	return problems;
}

/// @brief Print out the solutions:
void
print_solutions(
	std::string const & appname,
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingProblem_API const & problem,
	std::vector< seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingSolution_APICSP > const & solutions
) {
	using masala::base::Size;

	tracerman->write_to_tracer( appname, "Produced " + std::to_string( solutions.size() ) + " solutions." );

	std::string const separator( "========================================" );
	tracerman->write_to_tracer( appname, separator );
	tracerman->write_to_tracer( appname, "PROBLEM:" );
	problem.print_problem();
	tracerman->write_to_tracer( appname, separator );
	for( Size i(0); i<solutions.size(); ++i ) {
		tracerman->write_to_tracer( appname, "SOLUTION " + std::to_string(i) + ":" );
		tracerman->write_to_tracer( appname, separator );
		solutions[i]->print_solution( false );
		tracerman->write_to_tracer( appname, separator );
	}
}

/// @brief Write the solutions to disk:
void
write_solutions_to_disk(
	std::string const & appname,
	masala::base::managers::tracer::MasalaTracerManagerHandle tracerman,
	seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingProblem_API const & problem,
	std::vector< seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem::SeatingSolution_APICSP > const & solutions
) {
	using masala::base::Size;
	using namespace masala::base::managers::disk;

	MasalaDiskManagerHandle diskman( MasalaDiskManager::get_instance() );

	diskman->write_ascii_file( "problem_summary.txt", problem.get_problem_string() + "\n" );

	for( Size i(0); i<solutions.size(); ++i ) {
		std::stringstream ss;
		ss << "solution_" << std::setw(6) << std::setfill('0') << i << ".txt";

		diskman->write_ascii_file( ss.str(), solutions[i]->get_solution_string(false) + "\n" );
		tracerman->write_to_tracer( appname, "\tWrote file " + ss.str() + "." );
	}

	tracerman->write_to_tracer( appname, "Wrote " + std::to_string(solutions.size()) + " solutions to disk." );
}

/// @brief Program entry point:
int 
main(
	int argc,
	char * argv[]
) {
	using masala::base::Size;
	using namespace masala::base::managers::tracer;
	using namespace masala::base::managers::engine;
	using namespace masala::base::managers::threads;
	using namespace masala::numeric_api::auto_generated_api::optimization::cost_function_network;
	using namespace masala::numeric_api::base_classes::optimization::cost_function_network;
	using namespace seating_optimization_masala_plugins::seating_optimization_api::auto_generated_api::seating_problem;

	// Were options loaded?
	int help_indicated(0);
	int masala_plugins_found(0);
	int optimizer_name_specified(0);
	int classical_mc_steps_specified(0);
	int total_threads_specified(0);
	int classical_attempts_per_problem_specified(0);
	int solutions_to_store_per_problem_specified(0);
	int flattening_boltzmann_temperature_specified(0);
	int do_greedy_specified(0);
	int probfile_name_specified(0);
	int dwave_samples_specified(0);
	int dwave_annealing_time_specified(0);
	int dwave_use_layout_embedding_specified(0);
	int dwave_solver_name_specified(0);
	int dwave_onenode_penalty_cap_specified(0);
	int dwave_twonode_penalty_cap_specified(0);

	// Allowed optimizer names:
	std::vector< std::string > const allowed_optimizer_names{
		"HillFlatteningMonteCarloCostFunctionNetworkOptimizer",
		"MonteCarloCostFunctionNetworkOptimizer",
		"DWaveQuantumQUBOProblemOptimizer"
	};

	// Options that we will load:
	std::vector< std::string > masala_plugin_paths;
	std::string optimizer_name, probfile_name, dwave_solver_name;
	masala::base::Size classical_mc_steps( 1000000 );
	masala::base::Size total_threads( 1 );
	masala::base::Size classical_attempts_per_problem( 1 );
	masala::base::Size solutions_to_store_per_problem( 1 );
	masala::base::Size dwave_samples( 1000 );
	masala::base::Real flattening_boltzmann_temperature( 10.0 );
	masala::base::Real dwave_annelaing_time( 20.0 );
	masala::base::Real dwave_onenode_penalty_cap( 100.0 );
	masala::base::Real dwave_twonode_penalty_cap( 100.0 );
	bool do_greedy( true ), dwave_use_layout_embedding( true );

	// Masala tracer manager:
	MasalaTracerManagerHandle tracerman( MasalaTracerManager::get_instance() );
	std::string const appname( "seating_optimization_masala_plugins::applications::applications_apps::optimize_seating" );
	tracerman->write_to_tracer( appname, "Starting optimize_seating application." );
	tracerman->write_to_tracer( appname, "Application created 15 August 2025 by Vikram K. Mulligan, Biomolecular Design Group, "
		"Center for Computational Biology, Flatiron Institute, in collaboration with Karie A. Nicholas, Foundation for Health Care Quality."
	);
	tracerman->write_to_tracer( appname, "Please write to vmulligan@flatironinstitute.org for questions.");

	// Load options:
	if(
		!load_options(
			argc, argv, tracerman, appname,
			allowed_optimizer_names, masala_plugin_paths, optimizer_name,
			classical_mc_steps, total_threads, classical_attempts_per_problem,
			solutions_to_store_per_problem, flattening_boltzmann_temperature, do_greedy,
			probfile_name,
			help_indicated, masala_plugins_found, optimizer_name_specified,
			classical_mc_steps_specified, total_threads_specified, classical_attempts_per_problem_specified,
			solutions_to_store_per_problem_specified, flattening_boltzmann_temperature_specified, do_greedy_specified,
			probfile_name_specified,
			dwave_samples_specified, dwave_samples, dwave_annealing_time_specified, dwave_annelaing_time,
			dwave_use_layout_embedding_specified, dwave_use_layout_embedding, dwave_solver_name_specified, dwave_solver_name,
			dwave_onenode_penalty_cap_specified, dwave_onenode_penalty_cap, dwave_twonode_penalty_cap_specified, dwave_twonode_penalty_cap
		)
	) {
		return 0;
	}

	// Set threads:
	MasalaThreadManager::get_instance()->set_total_threads( total_threads );

	// Load masala plugins:
	load_masala_plugins( masala_plugin_paths );

	// Load the optimizer settings.  This fully configures the optimizer.
	CHECK_OR_THROW( optimizer_name_specified == 1, appname, "main", "An optimizer must be specified with the -optimizer_name flag." );
	MasalaEngineAPICSP optimizer_api(
		load_optimizer_settings(
			tracerman,
			appname,
			allowed_optimizer_names,
			optimizer_name,
			classical_mc_steps_specified, classical_mc_steps,
			classical_attempts_per_problem_specified, classical_attempts_per_problem,
			solutions_to_store_per_problem_specified, solutions_to_store_per_problem,
			flattening_boltzmann_temperature_specified, flattening_boltzmann_temperature,
			do_greedy_specified, do_greedy,
			dwave_samples_specified, dwave_samples, dwave_annealing_time_specified, dwave_annelaing_time,
			dwave_use_layout_embedding_specified, dwave_use_layout_embedding, dwave_solver_name_specified, dwave_solver_name,
			dwave_onenode_penalty_cap_specified, dwave_onenode_penalty_cap, dwave_twonode_penalty_cap_specified, dwave_twonode_penalty_cap
		)
	);

	// Load the problem specification:
	CHECK_OR_THROW( probfile_name_specified == 1, appname, "main", "A problem definition file must be specified with the -problem_file flag." );
	SeatingProblem_APICSP seating_problem( load_problem_specification( appname, probfile_name ) );

	// Generate the CFN problem:
	std::vector< std::vector< bool > > allowed_seats_by_guest;
	std::vector< std::map< Size, Size > > guest_choice_to_seat_index;
	std::vector< std::map< Size, Size > > seat_index_to_guest_choice;
	CostFunctionNetworkOptimizationProblems_APICSP problems(
		set_up_cfn_problem( appname, tracerman, *optimizer_api, *seating_problem,
			allowed_seats_by_guest, guest_choice_to_seat_index, seat_index_to_guest_choice
		)
	);

	// Solve the problem:
	std::vector< SeatingSolution_APICSP > solutions(
		solve_problem( appname, tracerman, *optimizer_api, seating_problem, *problems, guest_choice_to_seat_index )
	);

	// Print the solution(s):
	print_solutions( appname, tracerman, *seating_problem, solutions );

	// Write the solution(s) to disk:
	write_solutions_to_disk( appname, tracerman, *seating_problem, solutions );

	// Shutdown and cleanup:
	unload_masala_plugins();

	return 0;

}
