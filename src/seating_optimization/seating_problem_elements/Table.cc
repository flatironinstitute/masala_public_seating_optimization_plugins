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

/// @file src/seating_optimization/seating_problem_elements/Table.cc
/// @brief Implementations for a Table.
/// @details A Table is an object around which a bunch of Seat objects are arranged, at which several people may sit.
/// This is intended to be a non-instantiable base class for concrete derived classes defining various table shapes.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/Table.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Seat.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_TwoInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_ZeroInput.tmpl.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
Table::Table( Table const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	Table::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
Table::clone() const {
	return masala::make_shared< Table >( *this );
}

/// @brief Make a fully independent copy of this object.
TableSP
Table::deep_clone() const {
	TableSP new_table( std::static_pointer_cast< Table >( this->clone() ) );
	new_table->make_independent();
	return new_table;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem", "SeatingProblemElement", "Table" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
Table::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Table" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "table" }
std::vector< std::string >
Table::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"table",
	};
}

/// @brief Get the name of this class.
/// @returns "Table".
std::string
Table::class_name() const {
	return "Table";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
Table::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the Table class.
masala::base::api::MasalaObjectAPIDefinitionCWP
Table::get_api_definition() {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::api::getter;
	using masala::base::Real;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex() );

	if( api_definition() == nullptr ) {

		MasalaObjectAPIDefinitionSP api_def(
			masala::make_shared< MasalaObjectAPIDefinition >(
				*this,
				"The Table class stores all information associated with a table around which several guest seats could be arranged.  This "
				"is a base class not intended to be instantiated directly.  Derived classes are defined for particular shapes of table.",
				false, true
			)
		);
		ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( Table, api_def );

		// Work functions:

		// Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"x",
				"Get the x-coordinate of the table's centre.",
				"x", "The x-coordinate of the tables's centre, in meters.",
				false, false,
				std::bind( &Table::x, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"y",
				"Get the y-coordinate of the table's centre.",
				"y", "The y-coordinate of the tables's centre, in meters.",
				false, false,
				std::bind( &Table::y, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"angle",
				"Get the orientation of the table.  A table has an orientation, defined as the clockwise angle, in degrees, from facing "
				"north (the (0,1) direction in x-y space).",
				"angle_degrees", "The table's orientation, in degrees, defined as the clockwise angle from facing north (the (0,1) "
				"direction in x-y space).",
				false, false,
				std::bind( &Table::angle, this )
			)
		);

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Real const, Real const > >(
				"set_coordinates",
				"Set the coordinates of the centre of the table.  A table has coordinates in R^2 (x and y).",
				"x_in", "The x coordinate, in meters.",
				"y_in", "The y coordinate, in meters.",
				false, false,
				std::bind( &Table::set_coordinates, this, std::placeholders::_1, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Real const > >(
				"set_angle",
				"Set the table's angle.  A table has an orientation, defined as the clockwise angle, "
				"in degrees, from facing north (the (0,1) direction in x-y space).",
				"angle_in", "The input angle, in degrees.",
				false, false,
				std::bind( &Table::set_angle, this, std::placeholders::_1 )
			)
		);

		api_definition() = api_def; //Make const.
	}

	return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the x-coordinate of the centre of the table.
masala::base::Real
Table::x() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return x_;
}

/// @brief Get the y-coordinate of the centre of the table.
masala::base::Real
Table::y() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return y_;
}

/// @brief Get the orientation of the table.
/// @details A table has an orientation, defined as the clockwise angle, in degrees, from facing
/// north (the (0,1) direction in x-y space).	
masala::base::Real
Table::angle() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return angle_degrees_;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Set the coordinates of the table's centre.  A table has coordinates in R^2 (x and y).
void
Table::set_coordinates(
	Real const x_in,
	Real const y_in
) {
	std::lock_guard< std::mutex > lock( mutex() );
	x_ = x_in;
	y_ = y_in;
}

/// @brief Set the table's angle.
/// @details A table has an orientation, defined as the clockwise angle, in degrees, from facing
/// north (the (0,1) direction in x-y space).	
void
Table::set_angle(
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
Table::protected_make_independent() {
	// TODO DEEP CLONING

	for( Size i(0); i<seats_.size(); ++i ) {
		if( seats_[i] != nullptr ) {
			seats_[i] = std::static_pointer_cast< Seat >( seats_[i]->clone() );
			CHECK_OR_THROW_FOR_CLASS( seats_[i] != nullptr, "protected_make_independent", "Could not clone a Seat object." );
			seats_[i]->make_independent();
		}
	}

	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Table::protected_assign( SeatingElementBase const & src ) {
	Table const * const src_ptr_cast( dynamic_cast< Table const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a Table object."
	);
	
	x_ = src_ptr_cast->x_;
	y_ = src_ptr_cast->y_;
	angle_degrees_ = src_ptr_cast->angle_degrees_;
	seats_ = src_ptr_cast->seats_;

	Parent::protected_assign( src );
}

/// @brief Allow derived classes to access the seats vector.  This is expected to occur under mutex lock, but
/// this function does no mutex-locking.
std::vector< SeatSP > &
Table::protected_seats() {
	return seats_;
}

/// @brief Update the coordinates of seats on a change of table coordinates or dimensions.
/// @details Base class throws.  Derived classes should override this.
void
Table::protected_update_seat_coordinates() {
	MASALA_THROW( class_namespace() + "::" + class_name(), "protected_update_seat_coordinates", "This class has not implemented this function.  This is a "
		"program error which ought not to occur.  Please consult a developer."
	);
}

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins