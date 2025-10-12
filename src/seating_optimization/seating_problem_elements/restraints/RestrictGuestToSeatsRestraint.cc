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

/// @file src/seating_optimization/seating_problem_elements/restraints/RestrictGuestToSeatsRestraint.cc
/// @brief Implementations for a RestrictGuestToSeatsRestraint.
/// @details The RestrictGuestToTableRestraint class strictly limits a guest to a particular set of seats.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/restraints/RestrictGuestToSeatsRestraint.hh>

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
#include <base/utility/container/container_util.tmpl.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace restraints {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
RestrictGuestToSeatsRestraint::RestrictGuestToSeatsRestraint( RestrictGuestToSeatsRestraint const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	RestrictGuestToSeatsRestraint::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
RestrictGuestToSeatsRestraint::clone() const {
	return masala::make_shared< RestrictGuestToSeatsRestraint >( *this );
}

/// @brief Make a fully independent copy of this object.
RestrictGuestToSeatsRestraintSP
RestrictGuestToSeatsRestraint::deep_clone() const {
	RestrictGuestToSeatsRestraintSP new_guest( std::static_pointer_cast< RestrictGuestToSeatsRestraint >( this->clone() ) );
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
RestrictGuestToSeatsRestraint::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Restraint" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "restraint" }
std::vector< std::string >
RestrictGuestToSeatsRestraint::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"restraint",
	};
}

/// @brief Get the name of this class.
/// @returns "RestrictGuestToSeatsRestraint".
std::string
RestrictGuestToSeatsRestraint::class_name() const {
	return "RestrictGuestToSeatsRestraint";
}

/// @brief Get the namespace for this class.
/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints".
std::string
RestrictGuestToSeatsRestraint::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the RestrictGuestToSeatsRestraint class.
masala::base::api::MasalaObjectAPIDefinitionCWP
RestrictGuestToSeatsRestraint::get_api_definition() {
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
				"The RestrictGuestToSeatsRestraint class restricts a guest to only have seating options at a given set of seats.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( RestrictGuestToSeatsRestraint, api_def );

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line", "Configure this object from a line in an input file.  "
				"Base class implementation throws.  Must be overridden by derived classes.",
				"input_line", "The line from which we are configuring this object.  Syntax depends on "
				"derived class.  Must start with an identifier for the restraint type.",
				false, true,
				std::bind( &RestrictGuestToSeatsRestraint::configure_from_input_line, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"set_guest_uid", "Set the guest UID.",
				"setting", "The unique ID of the guest.",
				false, false,
				std::bind( &RestrictGuestToSeatsRestraint::set_guest_uid, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Size const > >(
				"add_seat", "Add a seat index.",
				"setting", "The seat index to add.",
				false, false,
				std::bind( &RestrictGuestToSeatsRestraint::add_seat, this, std::placeholders::_1 )
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
				"guest, where each entry is a Boolean vector with one entry per seat.  RestrictGuestToSeatsRestraints can set some subset of these Booleans "
				"to false.",
				true, false, false, true,
				"seating_problem", "The object describing the seats, tables, guests, and restraints.",
				"allowed_seating_choices_by_guest", "A vector of vectors of Booleans, where the outer vector is indexed by guest and the "
				"inner vector is indexed by seat global index.  True means that this guest can sit here, and false means they cannot.",
				"void", "This function returns nothing.",
				std::bind( &RestrictGuestToSeatsRestraint::restrain_seating_choices, this, std::placeholders::_1, std::placeholders::_2 )
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
RestrictGuestToSeatsRestraint::configure_from_input_line(
	std::string const & input_line
) {
	using masala::base::Size;
	std::lock_guard< std::mutex > lock( mutex() );
	std::istringstream ss(input_line);
	std::string temp;
	signed long int tempint;
	ss >> temp >> guest_uid_;
	CHECK_OR_THROW_FOR_CLASS( temp == class_name(), "configure_from_input_line", "Expected line to start with \"" + class_name() + "\", but got \"" + temp + "\"." );

	while( !ss.eof() ) {
		ss >> tempint;
		CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Unable to parse line \"" + input_line + "\"." );
		CHECK_OR_THROW_FOR_CLASS( tempint >= 0, "configure_from_input_line", "Expected non-negative table index." );
		CHECK_OR_THROW_FOR_CLASS( !masala::base::utility::container::has_value( seats_, static_cast<Size>(tempint) ),
			"configure_from_input_line", "The seat index " + std::to_string(tempint) + " has already been added."
		);
		seats_.push_back( static_cast<Size>(tempint) );
	}
}

/// @brief Set the guest UID.
void
RestrictGuestToSeatsRestraint::set_guest_uid(
	std::string const & setting
) {
	std::lock_guard< std::mutex > lock( mutex() );
	guest_uid_ = setting;
}

/// @brief Add a seat to the set to which we will restrict this guest.
void
RestrictGuestToSeatsRestraint::add_seat(
	masala::base::Size const setting
) {
	std::lock_guard< std::mutex > lock( mutex() );
	CHECK_OR_THROW_FOR_CLASS( !masala::base::utility::container::has_value( seats_, setting ), "add_seat", "Seat " + std::to_string(setting) + " was already added." );
	seats_.push_back(setting);
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Limit the allowed seating choices for the guests.
/// @details Base class implementation throws.  Must be overridden by derived classes.  The allowed_seating_choices_by_guest
/// vector is a vector with one entry per guest, where each entry is a Boolean vector with one entry per seat.  RestrictGuestToSeatsRestraints can
/// set some subset of these Booleans to false.
void
RestrictGuestToSeatsRestraint::restrain_seating_choices(
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
		if( !masala::base::utility::container::has_value(seats_, i) ) {
			innervec[i] = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
RestrictGuestToSeatsRestraint::protected_make_independent() {
	// Nothing to do here, since there's nothing to deep-clone.
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
RestrictGuestToSeatsRestraint::protected_assign( SeatingElementBase const & src ) {
	RestrictGuestToSeatsRestraint const * const src_ptr_cast( dynamic_cast< RestrictGuestToSeatsRestraint const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a RestrictGuestToSeatsRestraint object."
	);

	guest_uid_ = src_ptr_cast->guest_uid_;
	seats_ = src_ptr_cast->seats_;

	Parent::protected_assign( src );
}

} // namespace restraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
