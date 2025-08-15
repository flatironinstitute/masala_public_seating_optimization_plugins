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

/// @file src/seating_optimization/seating_problem_elements/Seat.cc
/// @brief Implementations for a Seat.
/// @details A Seat is a place where a person can sit.  It's defined by a coordinate in R^2 and
/// a direction (an angle, measured clockwise from north).
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/Seat.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_TwoInput.tmpl.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
Seat::Seat( Seat const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	Seat::protected_assign( src );
}


/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
Seat::clone() const {
	return masala::make_shared< Seat >( *this );
}

/// @brief Make a fully independent copy of this object.
SeatSP
Seat::deep_clone() const {
	SeatSP new_seat( std::static_pointer_cast< Seat >( this->clone() ) );
	new_seat->make_independent();
	return new_seat;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "OptimizationSolution", "CostFunctionNetworkOptimizationSolution", "Seat" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
Seat::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Seat" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "optimization_solution", "qubo_solution", "numeric" }
std::vector< std::string >
Seat::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"seat",
	};
}

/// @brief Get the name of this class.
/// @returns "Seat".
std::string
Seat::class_name() const {
	return "Seat";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
Seat::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the Seat class.
masala::base::api::MasalaObjectAPIDefinitionCWP
Seat::get_api_definition() {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using masala::base::Real;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex() );

	if( api_definition() == nullptr ) {

		MasalaObjectAPIDefinitionSP api_def(
			masala::make_shared< MasalaObjectAPIDefinition >(
				*this,
				"The Seat class stores all information associated with a seat at which a guest could be seated.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( Seat, api_def );

		// Work functions:

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Real const, Real const > >(
				"set_coordinates",
				"Set the seat's coordinates.  A seat has coordinates in R^2 (x and y).",
				"x_in", "The x coordinate, in meters.",
				"y_in", "The y coordinate, in meters.",
				false, false,
				std::bind( &Seat::set_coordinates, this, std::placeholders::_1, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Real const > >(
				"set_angle",
				"Set the seat's angle.  A seat has an orientation, defined as the clockwise angle, "
				"in degrees, from facing north (the (0,1) direction in x-y space).",
				"angle_in", "The input angle, in degrees.",
				false, false,
				std::bind( &Seat::set_angle, this, std::placeholders::_1 )
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

/// @brief Set the seat's coordinates.
void
Seat::set_coordinates(
	Real const x_in,
	Real const y_in
) {
	std::lock_guard< std::mutex > lock( mutex() );
	x_ = x_in;
	y_ = y_in;
}

/// @brief Set the seat's angle.
/// @details A seat has an orientation, defined as the clockwise angle, in degrees, from facing
/// north (the (0,1) direction in x-y space).	
void
Seat::set_angle(
	Real const angle_degrees_in
) {
	std::lock_guard< std::mutex > lock( mutex() );
	angle_degrees_ = masala::numeric_api::utility::angles::positive_angle_degrees( angle_degrees_in );
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Seat::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Seat::protected_assign( SeatingElementBase const & src ) {
	Seat const * const src_ptr_cast( dynamic_cast< Seat const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a Seat object."
	);
	// TODO ASSIGNMENT.

	Parent::protected_assign( src );
}
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins