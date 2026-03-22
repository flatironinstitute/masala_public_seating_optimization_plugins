/*
    Seating Optimization Masala Plugins 
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

/// @file src/seating_optimization/seating_problem/SeatingProblem.cc
/// @brief Implementations for SeatingProblem.
/// @details The SeatingProblem describes a seating problem, including the tables, the seats, the guests, and the constraints.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem/SeatingProblem.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Guest.hh>
#include <seating_optimization/seating_problem_elements/Table.hh>
#include <seating_optimization/seating_problem_elements/Seat.hh>
#include <seating_optimization/seating_problem_elements/Room.hh>
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>
#include <seating_optimization/seating_problem_elements/constraints/Constraint.hh>
#include <seating_optimization/seating_problem_elements/restraints/Restraint.hh>
#include <seating_optimization/seating_problem/SeatingSolution.hh>

// Base headers:
#include <base/utility/string/string_manipulation.hh>
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_OneInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ZeroInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_TwoInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_FourInput.tmpl.hh>
#include <base/managers/plugin_module/MasalaPluginAPI.hh>
#include <base/managers/plugin_module/MasalaPluginModuleManager.hh>

// Numeric headers:
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.hh>

// Numeric API headers:
#include <numeric_api/base_classes/optimization/cost_function_network/PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem_API.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationSolution_API.hh>

// STL headers:
#include <sstream>
#include <iomanip>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
SeatingProblem::SeatingProblem( SeatingProblem const & src ) :
    Parent( src )
{
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    SeatingProblem::protected_assign( src );
}

/// @brief Assignment operator.  Explicit due to mutex.
SeatingProblem &
SeatingProblem::operator=(
    SeatingProblem const & src
) {
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    protected_assign( src );
    return *this;
}


/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblemSP
SeatingProblem::clone() const {
    return masala::make_shared< SeatingProblem >( *this );
}

/// @brief Make a fully independent copy of this object.
SeatingProblemSP
SeatingProblem::deep_clone() const {
    SeatingProblemSP new_seat( std::static_pointer_cast< SeatingProblem >( this->clone() ) );
    new_seat->make_independent();
    return new_seat;
}

/// @brief Make this object fully independent by deep-cloning all of its sub-objects.
void
SeatingProblem::make_independent() {
    std::lock_guard< std::mutex > lockthis( mutex_ );
    protected_make_independent();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
SeatingProblem::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem" }
std::vector< std::string >
SeatingProblem::get_keywords() const {
	return std::vector< std::string > {
        "seating_problem"
	};
}

/// @brief Get the name of this class.
/// @returns "SeatingProblem".
std::string
SeatingProblem::class_name() const {
    return "SeatingProblem";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
SeatingProblem::class_namespace() const {
    return "seating_optimization_masala_plugins::seating_optimization::seating_problem";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the SeatingProblem class.
masala::base::api::MasalaObjectAPIDefinitionCWP
SeatingProblem::get_api_definition() {
    using namespace masala::base::api;
    using namespace masala::base::api::setter;
    using namespace masala::base::api::getter;
    using namespace masala::base::api::work_function;
    using masala::base::Real;
    using masala::base::Size;

    std::lock_guard< std::mutex > lock( mutex_ );

    if( api_definition() == nullptr ) {

        MasalaObjectAPIDefinitionSP api_def(
            masala::make_shared< MasalaObjectAPIDefinition >(
                *this,
                "The SeatingProblem describes a seating problem, including the tables, the seats, the guests, and the constraints.",
                false, false
            )
        );
        ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( SeatingProblem, api_def );

        // Work functions:
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< std::vector< std::pair< masala::base::Size, masala::base::Size > > > >(
				"get_adjacent_seat_global_indices",
				"Get a vector of pairs of global seat indices, with one pair for every two seats that are next to one another at "
				"each table in the problem.",
				true, false, false, false,
				"adjacent_seat_global_indices",
				"A vector of pairs representing the global, zero-based indices of the seats that are adjacent to one another.",
				std::bind( &SeatingProblem::get_adjacent_seat_global_indices, this )
			)
		);
		api_def->add_work_function(
			masala::make_shared<
				MasalaObjectAPIWorkFunctionDefinition_FourInput<
					void,
					masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem_API &,
					std::vector< std::vector< bool > > &,
					std::vector< std::map< masala::base::Size,  masala::base::Size > > &,
					std::vector< std::map< masala::base::Size,  masala::base::Size > > &
				>
			>(
				"set_up_cfn_problem", "Configure and finalize a cost function network optimization problem from this object.",
				true, false, false, false,
				"problem", "A shared pointer to an empty CFN problem instance.  Filled and finalized by this operation.",
				"allowed_seats", "An empty vector of Boolean vectors.  Filled by this operation to produce a vector indexed by guest index, containing "
				"vectors indexed by seat index, indicating whether each guest can sit at each seat or not.",
				"guest_choice_to_seat_index", "An empty vector of Size-Size maps.  Filled by this operation to produce a vector indexed by guest index, "
				"containing maps of guest choice index to absolute seat index.",
				"seat_index_to_guest_choice", "An empty vector of Size-Size maps.  Filled by this operation to produce a vector indexed by guest index, "
				"containing maps absolute seat index to guest choice index.",
				"void", "This function returns nothing.",
				std::bind( &SeatingProblem::set_up_cfn_problem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_TwoInput< SeatingSolutionSP, masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationSolution_API const &, std::vector< std::map< masala::base::Size, masala::base::Size > > const & > >(
				"seating_solution_from_cfn_solution", "Given a CFN solution, generate a SeatingSolution object from it.  "
				"The returned object is unfinalized, since it needs a shared pointer from this SeatingProblem object to be "
				"cached in it.",
				true, false, false, false,
				"cfn_solution", "The solution to a cost function network optimization problem.",
				"guest_choice_to_seat_index", "a vector indexed by guest index, containing maps of guest choice index to absolute seat index.",
				"seating_solution", "A solution to this seating problem.",
				std::bind( &SeatingProblem::seating_solution_from_cfn_solution, this, std::placeholders::_1, std::placeholders::_2 )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< void > >(
				"finalize", "Indicate that this object is fully set up.",
				false, false, false, false,
				"void", "This function returns nothing.",
				std::bind( &SeatingProblem::finalize, this )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_OneInput< bool, Size const > >(
				"seat_is_at_a_table", "Given a global seat index, determine whether this seat is at a table.",
				true, false, false, false,
				"seat_index", "The global index of the seat.",
				"seat_is_at_a_table", "True if this seat is at a table; false if it is free-floating.",
				std::bind( &SeatingProblem::seat_is_at_a_table, this, std::placeholders::_1 )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_OneInput< std::vector< Size >, Size const > >(
				"seats_at_table", "Get a vector of global seat indices for the seats at a given table.",
				true, false, false, false,
				"table_index", "The index of the table.",
				"seats_at_table", "A vector of the global indices of the seats at this table.",
				std::bind( &SeatingProblem::seats_at_table, this, std::placeholders::_1 )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_OneInput< std::pair< Size, Size >, Size const > >(
				"table_and_local_seat_index_from_global_seat_index", "Given a global seat index, determine the table index "
				"and local index of the seat at the table.  Throws if the seat isn't at a table.",
				true, false, false, false,
				"seat_index", "The global index of the seat.",
				"table_and_local_seat_index", "A pair, where the first entry is the table index, and the second is the local "
				"index of the seat at the table.",
				std::bind( &SeatingProblem::table_and_local_seat_index_from_global_seat_index, this, std::placeholders::_1 )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< void > >(
				"print_problem", "Print a description of the problem to the tracer.  Problem must be finalized.",
				true, false, false, false,
				"void", "This function returns nothing.",
				std::bind( &SeatingProblem::print_problem, this )
			)
		);
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< std::string > >(
				"get_problem_string", "Print a description of the problem to a string.  Problem must be finalized.",
				true, false, false, false,
				"problem_string", "A description of the problem, in string form.",
				std::bind( &SeatingProblem::get_problem_string, this )
			)
		);

        // Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_OneInput< Size, std::string const & > >(
				"guest_index_from_uid", "Given a unique guest identifier, get the guest index.  Throws if guest not found.  Indices are zero-based.",
				"guest_unique_identifier", "The unique identifier for the guest in question.",
				"guest_index", "The zero-based index (number) for the guest.",
				false, false,
				std::bind( &SeatingProblem::guest_index_from_uid, this, std::placeholders::_1 )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Size > >(
				"n_guests", "Get the number of guests.",
				"n_guests", "The number of guests.",
				false, false,
				std::bind( &SeatingProblem::n_guests, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Size > >(
				"n_seats", "Get the number of seats.",
				"n_seats", "The number of seats.",
				false, false,
				std::bind( &SeatingProblem::n_seats, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Size > >(
				"n_tables", "Get the number of tables.",
				"n_tables", "The number of tables.",
				false, false,
				std::bind( &SeatingProblem::n_tables, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< bool > >(
				"finalized", "Has this object been finalized?",
				"finalized", "True if this object has been finalized; false otherwise.",
				false, false,
				std::bind( &SeatingProblem::finalized, this )
			)
		);

        // Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::vector< std::string > const & > >(
				"configure_from_problem_definition_file_lines", "Configure from the lines of a file.",
				"filelines", "The lines of the file, as a vector of strings.",
				false, false,
				std::bind( &SeatingProblem::configure_from_problem_definition_file_lines, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & > >(
				"add_guest", "Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.",
				"guest_in", "The guest to add.",
				false, false,
				std::bind( &SeatingProblem::add_guest, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP const & > >(
				"add_table", "Add a table.  Stored directly; not cloned.",
				"table_in", "The table to add.",
				false, false,
				std::bind( &SeatingProblem::add_table, this, std::placeholders::_1 )
			)
		);

        api_definition() = api_def; //Make const.
    }

    return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Given a unique guest identifier, get the guest index.  Throws if guest not found.  Indices are zero-based.
masala::base::Size
SeatingProblem::guest_index_from_uid(
	std::string const & guest_unique_identifier
) const {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	std::lock_guard< std::mutex > lock( mutex_ );
	std::map< std::string, std::pair< Size, GuestCSP > >::const_iterator it( guests_.find( guest_unique_identifier ) );
	CHECK_OR_THROW_FOR_CLASS( it != guests_.end(), "guest_index_from_uid", "No unique guest ID \"" + guest_unique_identifier + "\" was found." );
	return it->second.first;
}

/// @brief Get the number of guests.
masala::base::Size
SeatingProblem::n_guests() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return guests_.size();
}

/// @brief Get the number of seats.
masala::base::Size
SeatingProblem::n_seats() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return seat_indices_.size();
}

/// @brief Get the number of tables.
masala::base::Size
SeatingProblem::n_tables() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return tables_.size();
}

/// @brief Access a guest, by guest index.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP
SeatingProblem::guest(
	masala::base::Size const guest_index
) const {
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( finalized_, "guest", "This object must be finalized before calling this function." );
	CHECK_OR_THROW_FOR_CLASS( guest_index < guests_by_index_.size(), "guest", "The guest index " + std::to_string(guest_index)
		+ " is out of range.  Only " + std::to_string(guests_by_index_.size()) + " guests have been defined."
	);
	return guests_by_index_.at(guest_index);
}

/// @brief Access a seat, by seat index.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP
SeatingProblem::seat(
	masala::base::Size const seat_index
) const {
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( finalized_, "seat", "This object must be finalized before calling this function." )
	CHECK_OR_THROW_FOR_CLASS( seat_index < seats_by_index_.size(), "guest", "The seat index " + std::to_string(seat_index)
		+ " is out of range.  Only " + std::to_string(seats_by_index_.size()) + " seats have been defined."
	);
	return seats_by_index_.at(seat_index);
}

/// @brief Has this object been finalized?
bool
SeatingProblem::finalized() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return finalized_;
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Configure from the lines of a file.
void
SeatingProblem::configure_from_problem_definition_file_lines(
	std::vector< std::string > const & file_lines
) {
	using namespace masala::base::managers::plugin_module;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;
	
	MasalaPluginModuleManagerHandle plugman( MasalaPluginModuleManager::get_instance() );

	std::lock_guard< std::mutex > lock(mutex_);
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "configure_from_problem_definition_file_lines", "This object must not be finalized before this function is called." );

	for( std::string const & line : file_lines ) {
		std::string const line_sans_comments( line.substr(0, line.find_first_of('#') ) );
		std::string const linestripped( masala::base::utility::string::trim( line_sans_comments ) );
		if( !linestripped.empty() ) {
			write_to_tracer( "=== Parsing \"" + linestripped + "\". ===" );
			std::istringstream ss( linestripped );
			std::string firstname;
			ss >> firstname;
			CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_problem_definition_file_lines", "Could not parse line \"" + linestripped + "\"." );
			
			MasalaPluginAPISP plugin_api(
				plugman->create_plugin_object_instance_by_short_name(
					{ "SeatingProblem", "SeatingProblemElement"}, firstname, true
				)
			);
			CHECK_OR_THROW_FOR_CLASS( plugin_api != nullptr, "configure_from_problem_definition_file_lines",
				"Could not create an object of type \"" + firstname + "\".  Check input for typos."
			);
			SeatingElementBaseSP seating_element( std::dynamic_pointer_cast< SeatingElementBase >( plugin_api->get_inner_plugin_object_nonconst() ) );
			CHECK_OR_THROW_FOR_CLASS( seating_element != nullptr, "configure_from_problem_definition_file_lines",
				"Could not create a SeatingElement of type \"" + plugin_api->inner_class_name() + "\".  Check input "
				"for typos, and that the class in question is a SeatingElementBase derived class."
			);

			seating_element->configure_from_input_line( linestripped );
			write_to_tracer( "Configured a \"" + seating_element->class_name() + "\" object." );

			// Determine the type of the object, and store it appropriately.
			GuestCSP guest( std::dynamic_pointer_cast< Guest const >(seating_element) );
			if( guest != nullptr ) { protected_add_guest(guest); }
			else {
				restraints::RestraintCSP restraint( std::dynamic_pointer_cast< restraints::Restraint const >(seating_element) );
				if( restraint != nullptr ) { protected_add_restraint(restraint); }
				else {
					constraints::ConstraintCSP constraint( std::dynamic_pointer_cast< constraints::Constraint const >(seating_element) );
					if( constraint != nullptr ) { protected_add_constraint(constraint); }
					else {			
						TableCSP table( std::dynamic_pointer_cast< Table const >(seating_element) );
						if( table != nullptr ) { protected_add_table(table); }
						else {
							SeatCSP seat( std::dynamic_pointer_cast< Seat const >(seating_element) );
							if( seat != nullptr ) { protected_add_seat(seat); }
							else {
								RoomCSP room( std::dynamic_pointer_cast< Room const >(seating_element) );
								if( room != nullptr ) { protected_add_room(room); }
								else {
									MASALA_THROW( class_namespace() + "::" + class_name(), "configure_from_problem_definition_file_lines",
										"Could not interpret \"" + seating_element->class_name() + "\" object as a Guest, a Table, a Seat, a Constraint, or a Restraint."
									);
								}
							}
						}
					}
				}
			}
		}
	}
}

/// @brief Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.
void
SeatingProblem::add_guest(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & guest_in
) {
	std::lock_guard< std::mutex > lock( mutex_ );
	protected_add_guest(guest_in);
}

/// @brief Add a table.  Stored directly; not cloned.
void
SeatingProblem::add_table(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP const & table_in
) {
	std::lock_guard< std::mutex > lock( mutex_ );
	protected_add_table(table_in);
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a vector of pairs of global seat indices, with one pair for every two seats that are next to one another
/// at each table in the problem.
std::vector< std::pair< masala::base::Size, masala::base::Size > >
SeatingProblem::get_adjacent_seat_global_indices() const {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;
	std::vector< std::pair< Size, Size > > outvec;
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( finalized_, "get_adjacent_seat_global_indices", "This object must be finalized before this function is called." );
	
	for( Size i(0); i<tables_.size(); ++i ) {
		std::vector< std::pair< SeatCSP, SeatCSP > > const adjacent_seats_for_table( tables_[i]->get_adjacent_seats() );
		for( auto const & seatpair : adjacent_seats_for_table ) {
			Size seat1_index( seat_indices_.at(seatpair.first) );
			Size seat2_index( seat_indices_.at(seatpair.second) );
			if( seat1_index > seat2_index ) { std::swap( seat1_index, seat2_index ); }
			outvec.push_back( std::make_pair(seat1_index, seat2_index) );
		}
	}

	outvec.shrink_to_fit();
	return outvec;
}

/// @brief Configure a cost function network optimization problem from this object.
/// @param[in] problem A shared pointer to an empty problem.  Filled and finalized by this operation.
void
SeatingProblem::set_up_cfn_problem(
	masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem_API & problem,
	std::vector< std::vector< bool > > & allowed_seats,
	std::vector< std::map< masala::base::Size,  masala::base::Size > > & guest_choice_to_seat_index,
	std::vector< std::map<  masala::base::Size,  masala::base::Size > > &seat_index_to_guest_choice
) const {
	using masala::base::Size;
	using namespace masala::numeric::optimization::cost_function_network;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints;

	std::vector< ConstraintCSP > constraints_copy;
	std::vector< RestraintCSP > restraints_copy;
	{
		std::lock_guard< std::mutex > lock( mutex_ );
		CHECK_OR_THROW_FOR_CLASS( finalized_, "set_up_cfn_problem", "This object must be finalized before this function is called." );
		constraints_copy = constraints_ ;
		restraints_copy = restraints_ ;
	}

	CostFunctionNetworkOptimizationProblemSP inner_problem( std::dynamic_pointer_cast< CostFunctionNetworkOptimizationProblem >( problem.get_inner_data_representation_object() ) );
	CHECK_OR_THROW_FOR_CLASS( inner_problem != nullptr, "set_up_cfn_problem", "Could not interpret inner object of \"" + problem.class_name() + "\" class as a CostFunctionNetworkOptimizationProblem." );
	CHECK_OR_THROW_FOR_CLASS( inner_problem->empty(), "set_up_cfn_problem", "A non-empty \"" + inner_problem->class_name() + "\" problem instance was passed to this function." );
	
	allowed_seats.clear();
	allowed_seats.resize( guests_.size(), std::vector<bool>( seat_indices_.size(), true ) );
	for( auto const & restraint : restraints_copy ) {
		restraint->restrain_seating_choices( *this, allowed_seats );
	}
	guest_choice_to_seat_index.clear();
	seat_index_to_guest_choice.clear();
	guest_choice_to_seat_index.resize( guests_.size() );
	seat_index_to_guest_choice.resize( guests_.size() );

	write_to_tracer( "Allowed seats by guest:" );
	for( Size i(0); i<guests_.size(); ++i ) {
		std::ostringstream ss;
		ss << "Guest " << i << " (" << guests_by_index_.at(i)->name() << "):\t";
		bool first(true);
		Size counter(0);
		for( Size j(0); j<allowed_seats[i].size(); ++j ) {
			if(allowed_seats[i][j]) {
				if(first) {
					first = false;
				} else {
					ss << ",";
				}
				ss << j;

				guest_choice_to_seat_index[i][counter] = j;
				seat_index_to_guest_choice[i][j] = counter;
				++counter;
			}
		}
		write_to_tracer( ss.str() );
	}

	// "Prime" the solutions object: ensure that each choice starts with a one-body penalty of zero.
	prime_choices( guest_choice_to_seat_index, *inner_problem );

	for( auto const & constraint : constraints_copy ) {
		constraint->add_constraint_to_cfn_problem( *this, allowed_seats, seat_index_to_guest_choice, *inner_problem, 1.0 );
	}

	// Finalize:
	problem.finalize();
}

/// @brief Given a CFN solution, generate a SeatingSolution object from it.
/// @note The returned object is unfinalized, since it needs a shared pointer from this SeatingProblem object to be cached in it.
SeatingSolutionSP
SeatingProblem::seating_solution_from_cfn_solution(
	masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationSolution_API const & cfn_solution,
	std::vector< std::map< masala::base::Size, masala::base::Size > > const & guest_choice_to_seat_index
) const {
	using namespace masala::numeric_api::auto_generated_api::optimization::cost_function_network;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex_ );
	CostFunctionNetworkOptimizationProblem_APICSP cfn_problem(
		std::static_pointer_cast< CostFunctionNetworkOptimizationProblem_API const >( cfn_solution.problem() )
	);
	CHECK_OR_THROW_FOR_CLASS( cfn_problem != nullptr, "seating_solution_from_cfn_solution", "Could not get CFN problem description." );
	CHECK_OR_THROW_FOR_CLASS( cfn_problem->total_nodes() == guests_.size(), "seating_solution_from_cfn_solution", "The number of guests ("
		+ std::to_string(guests_.size()) + ") did not match the number of nodes in the CFN problem (" + std::to_string(cfn_problem->total_nodes()) + ")."
	);

	SeatingSolutionSP seating_soln( masala::make_shared< SeatingSolution >() );

	std::vector< Size > const cfn_soln_all_positions( cfn_solution.solution_at_all_positions() );
	CHECK_OR_THROW_FOR_CLASS( cfn_soln_all_positions.size() == guests_.size(), "seating_solution_from_cfn_solution", "Size mismatch between CFN solution at all positions and guest count." );
	
	for( Size absnode_index(0), n_abs_nodes(cfn_soln_all_positions.size()); absnode_index < n_abs_nodes; ++absnode_index ) {
		seating_soln->add_guest_seat_assignment( absnode_index, guest_choice_to_seat_index[absnode_index].at(cfn_soln_all_positions[absnode_index]) );
	}

	return seating_soln;
}

/// @brief Given a global seat index, determine whether this seat is at a table.
bool
SeatingProblem::seat_is_at_a_table(
	masala::base::Size const seat_index
) const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return protected_seat_is_at_a_table( seat_index );
}

/// @brief Get a vector of global seat indices for the seats at a given table.
/// @param[in] table_index The index of the table.
std::vector< masala::base::Size >
SeatingProblem::seats_at_table(
	masala::base::Size const table_index
) const {
	using masala::base::Size;
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( table_index < tables_.size(), "seats_at_table", "Table index is out of range." );
	Size nseat( tables_[table_index]->num_seats() );
	std::vector< Size > outvec( nseat );
	for( Size i(0); i<nseat; ++i ){
		outvec[i] = seat_indices_.at( tables_[table_index]->seat(i) );
	}
	outvec.shrink_to_fit();
	return outvec;
}

/// @brief Given a global seat index, determine the table index and local index of the seat at the table.  Throws if the seat isn't at a table.
std::pair< masala::base::Size, masala::base::Size >
SeatingProblem::table_and_local_seat_index_from_global_seat_index(
	masala::base::Size const seat_index
) const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return protected_table_and_local_seat_index_from_global_seat_index( seat_index );
}

/// @brief Indicate that this object is fully set up.
void
SeatingProblem::finalize() {
	using masala::base::Size;
	std::lock_guard< std::mutex > lock( mutex_ );

	CHECK_OR_THROW_FOR_CLASS( !finalized_, "finalize", "This object has already been finalized." );
	finalized_ = true; 
}

/// @brief Print the problem to the tracer.  Problem must be finalized, or this function throws.
void
SeatingProblem::print_problem() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	write_to_tracer( protected_print_problem_to_string() );
}

/// @brief Print the problem to a string.  Problem must be finalized, or this function throws.
std::string
SeatingProblem::get_problem_string() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return protected_print_problem_to_string();
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Print the problem to a string.  Problem must be finalized, or this function throws.
/// @details This should be called from a mutex-locked context only.
std::string
SeatingProblem::protected_print_problem_to_string() const {
	std::stringstream outstream;

	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	CHECK_OR_THROW_FOR_CLASS( finalized_, "protected_print_problem_to_string", "The problem must be finalized before this function is called." );

	outstream << "GUESTS:\n";
	outstream << "Guest_index\tGuest_UID\tGuest_name\n";
	Size const nguests( guests_.size() );
	for( Size iguest(0); iguest<nguests; ++iguest ) {
		Guest const & guest( *guests_by_index_.at(iguest) );
		outstream << iguest
			<< "\t" << guest.unique_identifier()
			<< "\t\"" << guest.name() + "\"\n";
	}
	outstream << "\n";

	outstream << "ROOMS:\n";
	outstream << "Room_index\tRoom_type\tX\tY\tAngle\tType_specific_details\n";
	Size const nrooms( rooms_.size() );
	for( Size iroom(0); iroom < nrooms; ++iroom ) {
		Room const & room( *rooms_[iroom] );
		std::ostringstream ss;
		ss << std::setprecision(6);
		ss
			<< iroom
			<< "\t" << room.class_name()
			<< "\t" << room.x()
			<< "\t" << room.y()
			<< "\t" << room.angle()
			<< "\t" << room.type_specific_details_string();
		outstream << ss.str() << "\n";
	}
	outstream << "\n";

	outstream << "TABLES:\n";
	outstream << "Table_index\tTable_type\tX\tY\tAngle\tType_specific_details\n";
	Size const ntables( tables_.size() );
	for( Size itable(0); itable<ntables; ++itable ) {
		Table const & table( *tables_[itable] );
		std::ostringstream ss;
		ss << std::setprecision(6);
		ss
			<< itable
			<< "\t" << table.class_name()
			<< "\t" << table.x()
			<< "\t" << table.y()
			<< "\t" << table.angle()
			<< "\t" << table.type_specific_details_string();
		outstream << ss.str() << "\n";
	}
	outstream << "\n";

	outstream << "SEATS:\n";
	outstream << "Global_seat_index\tTable_index\tLocal_seat_index\tX\tY\tAngle\n";
	Size const nseats( seats_by_index_.size() );
	for( Size iseat(0); iseat<nseats; ++iseat ) {
		SeatCSP const & seat( seats_by_index_.at(iseat) );
		bool const is_at_table( protected_seat_is_at_a_table(iseat) );
		std::ostringstream ss;
		ss << iseat;
		if( is_at_table ) {
			std::pair< Size, Size > const table_and_local_seat( protected_table_and_local_seat_index_from_global_seat_index(iseat) );
			ss << "\t" << table_and_local_seat.first;
			ss << "\t" << table_and_local_seat.second;
		} else {
			ss << "\tN/A\tN/A";
		}
		ss << "\t" << seat->x() << "\t" << seat->y() << "\t" << seat->angle_degrees();
		outstream << ss.str();
		if( iseat < nseats-1 ) {
			outstream << "\n";
		}
	}

	return outstream.str();
}

/// @brief Given a global seat index, determine whether this seat is at a table.
/// @details Intended to be called from a mutex-locked context.
bool
SeatingProblem::protected_seat_is_at_a_table(
	masala::base::Size const seat_index
) const {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;
	std::map< Size, SeatCSP >::const_iterator it( seats_by_index_.find(seat_index) );
	CHECK_OR_THROW_FOR_CLASS( it != seats_by_index_.end(), "protected_seat_is_at_a_table", "Seat index was out of range." );
	for( auto const & table : tables_ ) {
		if( table->has_seat( it->second ) ) {
			return true;
		}
	}
	return false;
}

/// @brief Given a global seat index, determine the table index and local index of the seat at the table.  Throws if the seat isn't at a table.
/// @details Intended to be called from a mutex-locked context.
std::pair< masala::base::Size, masala::base::Size >
SeatingProblem::protected_table_and_local_seat_index_from_global_seat_index(
	masala::base::Size const seat_index
) const {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;
	std::map< Size, SeatCSP >::const_iterator it( seats_by_index_.find(seat_index) );
	CHECK_OR_THROW_FOR_CLASS( it != seats_by_index_.end(), "protected_table_and_local_seat_index_from_global_seat_index", "Seat index was out of range." );
	Size table_counter(0);
	for( auto const & table : tables_ ) {
		if( table->has_seat( it->second ) ) {
			return std::pair< Size, Size >( table_counter, table->seat_local_index( it->second ) );
		}
		++table_counter;
	}
	MASALA_THROW( class_namespace() + "::" + class_name(), "protected_table_and_local_seat_index_from_global_seat_index", "Seat does not appear to be associated with a table." );
}

/// @brief Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_guest(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & guest_in
) {
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "protected_add_guest", "This object must not be finalized before this function is called." );
	std::string const uid( guest_in->unique_identifier() );
	CHECK_OR_THROW_FOR_CLASS( guests_.count( uid ) == 0, "protected_add_guest", "A guest with unique identifier \""
		+ uid + "\" has already been added."
	);
	masala::base::Size nguests( guests_.size() );
	guests_[uid] = std::make_pair( nguests, guest_in );
	CHECK_OR_THROW_FOR_CLASS( guests_by_index_.count(nguests) == 0, "protected_add_guest", "Guest number " + std::to_string(nguests) + " has already been added." );
	guests_by_index_[nguests] = guest_in;
	write_to_tracer( "Added guest \"" + guest_in->name() + "\" (index " + std::to_string(nguests) + ") with unique identifier \"" + guest_in->unique_identifier() + "\"." );
}

/// @brief Add a table.  Stored directly; not cloned.
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_table(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP const & table_in
){
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "protected_add_table", "This object must not be finalized before this function is called." );
	for( auto const & table : tables_ ) {
		CHECK_OR_THROW_FOR_CLASS( table.get() != table_in.get(), "protected_add_table", "A table was passed to this function that has already been added!" );
	}
	tables_.push_back( table_in );
	
	regenerate_seat_indices();

	write_to_tracer( "Added table " + std::to_string(tables_.size() - 1) + " of type \"" + table_in->class_name()
		+ "\", at coordinates (" + std::to_string(table_in->x()) + ", " + std::to_string(table_in->y())
		+ "), with " + std::to_string( table_in->num_seats() ) + " seats." );
}

/// @brief Add a room.  Primarily for visualization
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_room(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::RoomCSP const & room_in
) {
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "protected_add_room", "This object must not be finalized before this function is called." );
	rooms_.push_back( room_in );
	write_to_tracer( "Added room " + std::to_string(rooms_.size() - 1) + " of type \"" + room_in->class_name()
		+ "\", at coordinates (" + std::to_string(room_in->x()) + ", " + std::to_string(room_in->y()) + ")."
	);
}

/// @brief Add a loose seat.  Stored directly; not cloned.  (NOT YET SUPPORTED -- THROWS.)
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_seat(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP const & //seat_in
) {
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "protected_add_seat", "This object must not be finalized before this function is called." );
	MASALA_THROW( class_namespace() + "::" + class_name(), "protected_add_seat", "Loose seats are not yet supported!" );
}

/// @brief Add a constraint.  Stored directly; not cloned.
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_constraint(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::ConstraintCSP const & constraint_in
) {
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "protected_add_constraint", "This object must not be finalized before this function is called." );
	constraints_.push_back( constraint_in );
	write_to_tracer( "Added constraint " + std::to_string(constraints_.size() -1) + " of type \"" + constraint_in->class_name() + "\"." );
}

/// @brief Add a restraint.  Stored directly; not cloned.
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_restraint(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::RestraintCSP const & restraint_in
) {
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "protected_add_restraint", "This object must not be finalized before this function is called." );
	restraints_.push_back( restraint_in );
	write_to_tracer( "Added restraint " + std::to_string(restraints_.size() -1) + " of type \"" + restraint_in->class_name() + "\"." );
}

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingProblem::protected_make_independent() {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints;

	api_definition_ = nullptr;
	if( !guests_.empty() ) {
		for( std::map< std::string, std::pair< Size, GuestCSP > >::iterator it( guests_.begin() ); it != guests_.end(); ++it ) {
			GuestSP new_guest( std::dynamic_pointer_cast< Guest >( it->second.second->clone() ) );
			CHECK_OR_THROW_FOR_CLASS( new_guest != nullptr, "protected_make_independent", "Could not clone guest \"" + it->second.second->name() + "\"." );
			new_guest->make_independent();
			it->second.second = new_guest;
			guests_by_index_[ it->second.first ] = new_guest;
		}
	}
	if( !tables_.empty() ) {
		for( Size i(0); i<tables_.size(); ++i ) {
			TableSP new_table( std::dynamic_pointer_cast< Table >( tables_[i]->clone() ) );
			CHECK_OR_THROW_FOR_CLASS( new_table != nullptr, "protected_make_independent", "Unable to clone table " + std::to_string(i) + "." );
			new_table->make_independent();
			tables_[i] = new_table;
		}
	}
	if( !rooms_.empty() ) {
		for( Size i(0); i<rooms_.size(); ++i ) {
			RoomSP new_room( std::dynamic_pointer_cast< Room >( rooms_[i]->clone() ) );
			CHECK_OR_THROW_FOR_CLASS( new_room != nullptr, "protected_make_independent", "Unable to clone room " + std::to_string(i) + "." );
			new_room->make_independent();
			rooms_[i] = new_room;
		}
	}
	if( !constraints_.empty() ) {
		for( Size i(0); i<constraints_.size(); ++i ) {
			ConstraintSP new_constraint( std::dynamic_pointer_cast< Constraint >( constraints_[i]->clone() ) );
			CHECK_OR_THROW_FOR_CLASS( new_constraint != nullptr, "protected_make_independent", "Unable to clone constraint " + std::to_string(i) + "." );
			new_constraint->make_independent();
			constraints_[i] = new_constraint;
		}
	}
	if( !restraints_.empty() ) {
		for( Size i(0); i<restraints_.size(); ++i ) {
			RestraintSP new_restraint( std::dynamic_pointer_cast< Restraint >( restraints_[i]->clone() ) );
			CHECK_OR_THROW_FOR_CLASS( new_restraint != nullptr, "protected_make_independent", "Unable to clone restraint " + std::to_string(i) + "." );
			new_restraint->make_independent();
			restraints_[i] = new_restraint;
		}
	}

	regenerate_seat_indices();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingProblem::protected_assign(
    SeatingProblem const & src
) {
	finalized_ = src.finalized_;
	guests_ = src.guests_;
	guests_by_index_ = src.guests_by_index_;
	tables_ = src.tables_;
	rooms_ = src.rooms_;
	seat_indices_ = src.seat_indices_;
	seats_by_index_ = src.seats_by_index_;
	constraints_ = src.constraints_;
	restraints_ = src.restraints_;
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Update the list of seat indices (from scratch).
/// @details The seat_indices_ map is cleared and repopulated by this operation.  No mutex locking
void
SeatingProblem::regenerate_seat_indices() {
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;
	using masala::base::Size;

	seat_indices_.clear();
	seats_by_index_.clear();
	Size counter(0);
	for( Size i(0); i<tables_.size(); ++i ) {
		for( Size j(0); j<tables_[i]->num_seats(); ++j ) {
			SeatCSP curseat(  tables_[i]->seat(j) );
			CHECK_OR_THROW_FOR_CLASS( seat_indices_.count(curseat) == 0, "regenerate_seat_indices", "A seat was already added to the seat_indices_ map." );
			seat_indices_[curseat] = counter;
			seats_by_index_[counter] = curseat;
			++counter;
		}
	}
}

/// @brief "Prime" all of the choices at all of the nodes to set their one-body penalties to zero.
void
SeatingProblem::prime_choices(
	std::vector< std::map< masala::base::Size,  masala::base::Size > > const & guest_choice_to_seat_index,
	masala::numeric::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem & problem
) const {
	using masala::base::Size;
	using namespace masala::numeric_api::base_classes::optimization::cost_function_network;

	PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem * problem_cast(
		dynamic_cast< PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem * >( &problem )
	);
	CHECK_OR_THROW_FOR_CLASS( problem_cast != nullptr, "prime_choices", "Could not interpret a CFN problem of "
		"type " + problem.class_name() + " as a PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem."
	);

	for( Size iguest(0); iguest <= guest_choice_to_seat_index.size(); ++iguest ){
		problem.set_minimum_number_of_choices_at_node( iguest, guest_choice_to_seat_index[iguest].size() );
		for( auto const & entry : guest_choice_to_seat_index[iguest] ) {
			problem_cast->set_onebody_penalty( iguest, entry.first, 0 );
		}
	}
}


} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins