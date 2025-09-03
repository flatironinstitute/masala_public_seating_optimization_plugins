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
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>
#include <seating_optimization/seating_problem_elements/constraints/Constraint.hh>

// Base headers:
#include <base/utility/string/string_manipulation.hh>
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_OneInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ZeroInput.tmpl.hh>
#include <base/managers/plugin_module/MasalaPluginAPI.hh>
#include <base/managers/plugin_module/MasalaPluginModuleManager.hh>

// STL headers:
#include <sstream>

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
                false, true
            )
        );
        ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( SeatingProblem, api_def );

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

        // Setters:
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
			GuestCSP guest( std::dynamic_pointer_cast< Guest const >(seating_element);
			if( guest != nullptr ) { protected_add_guest(guest); }
			else {			
				TableCSP table( std::dynamic_pointer_cast< Table const >(seating_element);
				if( table != nullptr ) { protected_add_table(table); }
				else {
					SeatCSP seat( std::dynamic_pointer_cast< Seat const >(seating_element) );
					if( seat != nullptr ) { protected_add_seat(seat); }
					else {
						MASALA_THROW( class_namespace() + "::" + class_name(), "configure_from_problem_definition_file_lines",
							"Could not interpret \"" + seating_element->class_name() + "\" object as a Guest, a Table, or a Seat."
						);
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

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_guest(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & guest_in
) {
	std::string const uid( guest_in->unique_identifier() );
	CHECK_OR_THROW_FOR_CLASS( guests_.count( uid ) == 0, "protected_add_guest", "A guest with unique identifier \""
		+ uid + "\" has already been added."
	);
	masala::base::Size nguests( guests_.size() );
	guests_[uid] = std::make_pair( nguests, guest_in );
}

/// @brief Add a table.  Stored directly; not cloned.
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_table(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP const & table_in
){
	for( auto const & table : tables_ ) {
		CHECK_OR_THROW_FOR_CLASS( table.get() != table_in.get(), "protected_add_table", "A table was passed to this function that has already been added!" );
	}
	tables_.push_back( table_in );
	
	regenerate_seat_indices();
}

/// @brief Add a loose seat.  Stored directly; not cloned.  (NOT YET SUPPORTED -- THROWS.)
/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
void
SeatingProblem::protected_add_seat(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP const & //table_in
) {
	MASALA_THROW( class_namespace() + "::" + class_name(), "protected_add_seat", "Loose seats are not yet supported!" );
}

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingProblem::protected_make_independent() {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	api_definition_ = nullptr;
	if( !guests_.empty() ) {
		for( std::map< std::string, std::pair< Size, GuestCSP > >::iterator it( guests_.begin() ); it != guests_.end(); ++it ) {
			GuestSP new_guest( std::dynamic_pointer_cast< Guest >( it->second.second->clone() ) );
			CHECK_OR_THROW_FOR_CLASS( new_guest != nullptr, "protected_make_independent", "Could not clone guest \"" + it->second.second->name() + "\"." );
			new_guest->make_independent();
			it->second.second = new_guest;
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
	regenerate_seat_indices();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingProblem::protected_assign(
    SeatingProblem const & src
) {
	guests_ = src.guests_;
	tables_ = src.tables_;
	seat_indices_ = src.seat_indices_;
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
	Size counter(0);
	for( Size i(0); i<tables_.size(); ++i ) {
		for( Size j(0); j<tables_[i]->num_seats(); ++j ) {
			SeatCSP curseat(  tables_[i]->seat(j) );
			CHECK_OR_THROW_FOR_CLASS( seat_indices_.count(curseat) == 0, "regenerate_seat_indices", "A seat was already added to the seat_indices_ map." );
			seat_indices_[curseat] = counter;
			++counter;
		}
	}
}


} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins