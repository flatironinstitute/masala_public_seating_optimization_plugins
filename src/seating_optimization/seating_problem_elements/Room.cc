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

/// @file src/seating_optimization/seating_problem_elements/Room.cc
/// @brief Implementations for a Room.
/// @details A Room is an object in which all the tables are arranged.  It is mainly used for visualization.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/Room.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Seat.hh>

// Base headers:
#include <base/utility/container/container_util.tmpl.hh>
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_TwoInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_ZeroInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ZeroInput.tmpl.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
Room::Room( Room const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	Room::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
Room::clone() const {
	return masala::make_shared< Room >( *this );
}

/// @brief Make a fully independent copy of this object.
RoomSP
Room::deep_clone() const {
	RoomSP newroom( std::static_pointer_cast< Room >( this->clone() ) );
	newroom->make_independent();
	return newroom;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem", "SeatingProblemElement", "Room" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
Room::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Room" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "room" }
std::vector< std::string >
Room::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"room",
	};
}

/// @brief Get the name of this class.
/// @returns "Room".
std::string
Room::class_name() const {
	return "Room";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
Room::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the Room class.
masala::base::api::MasalaObjectAPIDefinitionCWP
Room::get_api_definition() {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::api::getter;
	using namespace masala::base::api::work_function;
	using masala::base::Real;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex() );

	if( api_definition() == nullptr ) {

		MasalaObjectAPIDefinitionSP api_def(
			masala::make_shared< MasalaObjectAPIDefinition >(
				*this,
				"A Room is an object in which all the tables are arranged.  It is mainly used for visualization.  This "
				"is a base class not intended to be instantiated directly.  Derived classes are defined for particular "
				"shapes of room.",
				false, true
			)
		);
		ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( Room, api_def );

		// Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"x",
				"Get the x-coordinate of the room's centre.",
				"x", "The x-coordinate of the room's centre, in meters.",
				false, false,
				std::bind( &Room::x, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"y",
				"Get the y-coordinate of the room's centre.",
				"y", "The y-coordinate of the room's centre, in meters.",
				false, false,
				std::bind( &Room::y, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"angle",
				"Get the orientation of the room.  A room has an orientation, defined as the clockwise angle, in degrees, from facing "
				"north (the (0,1) direction in x-y space).",
				"angle_degrees", "The room's orientation, in degrees, defined as the clockwise angle from facing north (the (0,1) "
				"direction in x-y space).",
				false, false,
				std::bind( &Room::angle, this )
			)
		);

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Real const, Real const > >(
				"set_coordinates",
				"Set the coordinates of the centre of the room.  A room has coordinates in R^2 (x and y).",
				"x_in", "The x coordinate, in meters.",
				"y_in", "The y coordinate, in meters.",
				false, false,
				std::bind( &Room::set_coordinates, this, std::placeholders::_1, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Real const > >(
				"set_angle",
				"Set the room's angle.  A room has an orientation, defined as the clockwise angle, "
				"in degrees, from facing north (the (0,1) direction in x-y space).",
				"angle_in", "The input angle, in degrees.",
				false, false,
				std::bind( &Room::set_angle, this, std::placeholders::_1 )
			)
		);

		api_definition() = api_def; //Make const.
	}

	return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the x-coordinate of the centre of the room.
masala::base::Real
Room::x() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return x_;
}

/// @brief Get the y-coordinate of the centre of the room.
masala::base::Real
Room::y() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return y_;
}

/// @brief Get the orientation of the room.
/// @details A room has an orientation, defined as the clockwise angle, in degrees, from facing
/// north (the (0,1) direction in x-y space).	
masala::base::Real
Room::angle() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return angle_degrees_;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Set the coordinates of the room's centre.  A room has coordinates in R^2 (x and y).
void
Room::set_coordinates(
	Real const x_in,
	Real const y_in
) {
	std::lock_guard< std::mutex > lock( mutex() );
	protected_set_coordinates( x_in, y_in );
}

/// @brief Set the room's angle.
/// @details A room has an orientation, defined as the clockwise angle, in degrees, from facing
/// north (the (0,1) direction in x-y space).	
void
Room::set_angle(
	Real const angle_degrees_in
) {
	std::lock_guard< std::mutex > lock( mutex() );
	protected_set_angle( angle_degrees_in );
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
Room::protected_make_independent() {
	// TODO DEEP CLONING

	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Room::protected_assign( SeatingElementBase const & src ) {
	Room const * const src_ptr_cast( dynamic_cast< Room const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a Room object."
	);
	
	x_ = src_ptr_cast->x_;
	y_ = src_ptr_cast->y_;
	angle_degrees_ = src_ptr_cast->angle_degrees_;

	Parent::protected_assign( src );
}

/// @brief Allow derived classes to set coordinates.  Performs no mutex-locking; should be called from
/// a mutex-locked context only.
void
Room::protected_set_coordinates(
	masala::base::Real const x_in,
	masala::base::Real const y_in
) {
	x_ = x_in;
	y_ = y_in;
}

/// @brief Allow derived classes to set the angle.  Performs no mutex-locking; should be called from
/// a mutex-locked context only.  Angle is in degrees.
void
Room::protected_set_angle(
	masala::base::Real const angle_degrees_in
) {
	angle_degrees_ = masala::numeric_api::utility::angles::positive_angle_degrees( angle_degrees_in );
}

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins