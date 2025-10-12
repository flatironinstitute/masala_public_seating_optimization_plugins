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

/// @file src/seating_optimization/seating_problem_elements/restraints/RestrictGuestToTableRestraint.cc
/// @brief Implementations for a RestrictGuestToTableRestraint.
/// @details The RestrictGuestToTableRestraint class is the base class for restraints, which are elements of a seating problem.  They stricly limit the
/// seating choices for a particular guest, unlike Constraints, which only impose a penalty or a bonus for a particular seating choice.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/restraints/RestrictGuestToTableRestraint.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem/SeatingProblem.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_TwoInput.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace restraints {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
RestrictGuestToTableRestraint::RestrictGuestToTableRestraint( RestrictGuestToTableRestraint const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	RestrictGuestToTableRestraint::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
RestrictGuestToTableRestraint::clone() const {
	return masala::make_shared< RestrictGuestToTableRestraint >( *this );
}

/// @brief Make a fully independent copy of this object.
RestrictGuestToTableRestraintSP
RestrictGuestToTableRestraint::deep_clone() const {
	RestrictGuestToTableRestraintSP new_guest( std::static_pointer_cast< RestrictGuestToTableRestraint >( this->clone() ) );
	new_guest->make_independent();
	return new_guest;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem", "SeatingProblemElement", "Restraint" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
RestrictGuestToTableRestraint::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Restraint" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "restraint" }
std::vector< std::string >
RestrictGuestToTableRestraint::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"restraint",
	};
}

/// @brief Get the name of this class.
/// @returns "RestrictGuestToTableRestraint".
std::string
RestrictGuestToTableRestraint::class_name() const {
	return "RestrictGuestToTableRestraint";
}

/// @brief Get the namespace for this class.
/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints".
std::string
RestrictGuestToTableRestraint::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the RestrictGuestToTableRestraint class.
masala::base::api::MasalaObjectAPIDefinitionCWP
RestrictGuestToTableRestraint::get_api_definition() {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::api::work_function;
	using masala::base::Real;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex() );

	if( api_definition() == nullptr ) {

		MasalaObjectAPIDefinitionSP api_def(
			masala::make_shared< MasalaObjectAPIDefinition >(
				*this,
				"The RestrictGuestToTableRestraint class restricts a guest to only have seating options at a given table.",
				false, true
			)
		);
		ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( RestrictGuestToTableRestraint, api_def );

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line", "Configure this object from a line in an input file.  "
				"Base class implementation throws.  Must be overridden by derived classes.",
				"input_line", "The line from which we are configuring this object.  Syntax depends on "
				"derived class.  Must start with an identifier for the restraint type.",
				false, true,
				std::bind( &RestrictGuestToTableRestraint::configure_from_input_line, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"set_guest_uid", "Set the guest UID.",
				"setting", "The unique ID of the guest.",
				false, true,
				std::bind( &RestrictGuestToTableRestraint::set_guest_uid, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Size const > >(
				"set_table", "Set the table index.",
				"setting", "The table index.",
				false, true,
				std::bind( &RestrictGuestToTableRestraint::set_table, this, std::placeholders::_1 )
			)
		);

		// Work functions:
		api_def->add_work_function(
			masala::make_shared<
				MasalaObjectAPIWorkFunctionDefinition_TwoInput<
					void,
					seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const &,
					std::vector< std::vector< bool > > &
				>
			>(
				"restrain_seating_choices", "Limit the allowed seating choices for the guests.  Base class implementation throws.  "
				"Must be overridden by derived classes.  The allowed_seating_choices_by_guest vector is a vector with one entry per "
				"guest, where each entry is a Boolean vector with one entry per seat.  RestrictGuestToTableRestraints can set some subset of these Booleans "
				"to false.",
				true, false, false, true,
				"seating_problem", "The object describing the seats, tables, guests, and restraints.",
				"allowed_seating_choices_by_guest", "A vector of vectors of Booleans, where the outer vector is indexed by guest and the "
				"inner vector is indexed by seat global index.  True means that this guest can sit here, and false means they cannot.",
				"void", "This function returns nothing.",
				std::bind( &RestrictGuestToTableRestraint::restrain_seating_choices, this, std::placeholders::_1, std::placeholders::_2 )
			)
		);

		api_definition() = api_def; //Make const.
	}

	return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Configure this object from a line in an input file.
/// @details Base class implementation throws.  Must be overridden by derived classes.
void
RestrictGuestToTableRestraint::configure_from_input_line(
	std::string const & input_line
) {
	using masala::base::Size;
	std::lock_guard< std::mutex > lock( mutex() );
	std::istringstream ss(input_line);
	std::string temp;
	signed long int tempint;
	ss >> temp >> guest_uid_ >> tempint;
	CHECK_OR_THROW_FOR_CLASS( temp == class_name(), "configure_from_input_line", "Expected line to start with \"" + class_name() + "\", but got \"" + temp + "\"." );
	CHECK_OR_THROW_FOR_CLASS( tempint >= 0, "configure_from_input_line", "Expected non-negative table index." );
	table_ = static_cast<Size>( tempint );
}

/// @brief Set the guest UID.
void
RestrictGuestToTableRestraint::set_guest_uid(
	std::string const & setting
) {
	std::lock_guard< std::mutex > lock( mutex() );
	guest_uid_ = setting;
}

/// @brief Set the table index.
void
RestrictGuestToTableRestraint::set_table(
	masala::base::Size const setting
) {
	std::lock_guard< std::mutex > lock( mutex() );
	table_ = setting;
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Limit the allowed seating choices for the guests.
/// @details Base class implementation throws.  Must be overridden by derived classes.  The allowed_seating_choices_by_guest
/// vector is a vector with one entry per guest, where each entry is a Boolean vector with one entry per seat.  RestrictGuestToTableRestraints can
/// set some subset of these Booleans to false.
void
RestrictGuestToTableRestraint::restrain_seating_choices(
	seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & seating_problem,
	std::vector< std::vector< bool > > & allowed_seating_choices_by_guest
) const {
	using masala::base::Size;
	std::lock_guard< std::mutex > lock( mutex() );
	masala::base::Size guest_index( seating_problem.guest_index_from_uid(guest_uid_) );
	CHECK_OR_THROW_FOR_CLASS( guest_index < allowed_seating_choices_by_guest.size(), "restrain_seating_choices", "Expected allowed_seating_choices_by_guest "
		"array to be at least size " + std::to_string(guest_index+1) + ", but got array of size " + std::to_string( allowed_seating_choices_by_guest.size() )
		+ "."
	);
	std::vector< bool > & innervec( allowed_seating_choices_by_guest[guest_index] );
	for( Size i(0); i<innervec.size(); ++i ) {
		if( !seating_problem.seat_is_at_a_table(i) ) {
			innervec[i] = false;
		} else {
			if( seating_problem.table_and_local_seat_index_from_global_seat_index(i).first != table_ ) {
				innervec[i] = false;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
RestrictGuestToTableRestraint::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
RestrictGuestToTableRestraint::protected_assign( SeatingElementBase const & src ) {
	RestrictGuestToTableRestraint const * const src_ptr_cast( dynamic_cast< RestrictGuestToTableRestraint const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a RestrictGuestToTableRestraint object."
	);

	// TODO TODO TODO

	Parent::protected_assign( src );
}

} // namespace restraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
