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

/// @file src/seating_optimization/seating_problem_elements/RectangularRoom.cc
/// @brief Implementations for a RectangularRoom.
/// @details As the name suggests, a rectangular room is a room with a rectangular shape.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/RectangularRoom.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Seat.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_TwoInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_ZeroInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ZeroInput.tmpl.hh>
#include <base/utility/container/container_util.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// Numeric API headers:
#include <numeric_api/utility/constants/constants.hh>
#include <numeric_api/utility/angles/angle_util.hh>

// STL headers:
#include <sstream>
#include <cmath>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
RectangularRoom::RectangularRoom( RectangularRoom const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	RectangularRoom::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
RectangularRoom::clone() const {
	return masala::make_shared< RectangularRoom >( *this );
}

/// @brief Make a fully independent copy of this object.
RectangularRoomSP
RectangularRoom::deep_clone() const {
	RectangularRoomSP new_room( std::static_pointer_cast< RectangularRoom >( this->clone() ) );
	new_room->make_independent();
	return new_room;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem", "SeatingProblemElement", "Room", "RectangularRoom" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
RectangularRoom::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Room", "RectangularRoom" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "room", "rectangular" }
std::vector< std::string >
RectangularRoom::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"room",
		"rectangular"
	};
}

/// @brief Get the name of this class.
/// @returns "RectangularRoom".
std::string
RectangularRoom::class_name() const {
	return "RectangularRoom";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
RectangularRoom::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the RectangularRoom class.
masala::base::api::MasalaObjectAPIDefinitionCWP
RectangularRoom::get_api_definition() {
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
				"The RectangularRoom class is, as the name suggests, a room with a rectangular shape.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( RectangularRoom, api_def );

		// Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"x",
				"Get the x-coordinate of the room's centre.",
				"x", "The x-coordinate of the room's centre, in meters.",
				false, false,
				std::bind( &RectangularRoom::x, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"y",
				"Get the y-coordinate of the room's centre.",
				"y", "The y-coordinate of the room's centre, in meters.",
				false, false,
				std::bind( &RectangularRoom::y, this )
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
				std::bind( &RectangularRoom::angle, this )
			)
		);

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line",
				"Configure this object from a line in a problem definition file.  This implementation expects "
				"a line of the format 'RectangularRoom <xcoord> <ycoord> <angle_degrees> <length> <width>'.",
				"file_line", "A line from a configuration file.  Should start with the class name.",
				false, true,
				std::bind( &RectangularRoom::configure_from_input_line, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Real const, Real const > >(
				"set_coordinates",
				"Set the coordinates of the centre of the table.  A table has coordinates in R^2 (x and y).",
				"x_in", "The x coordinate, in meters.",
				"y_in", "The y coordinate, in meters.",
				false, false,
				std::bind( &RectangularRoom::set_coordinates, this, std::placeholders::_1, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Real const > >(
				"set_angle",
				"Set the table's angle.  A table has an orientation, defined as the clockwise angle, "
				"in degrees, from facing north (the (0,1) direction in x-y space).  Note that for a circular table, the angle "
				"defines the rotation of the chairs around the centre of the table.",
				"angle_in", "The input angle, in degrees.",
				false, false,
				std::bind( &RectangularRoom::set_angle, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Real const, Real const > >(
				"set_dimensions",
				"Set the dimensions of the room.",
				"length_in", "The length of the table, in meters (in the north-south direction prior to rotation).  Defaults to 1.0.",
				"width_in", "The width of the table, in meters (in the east-west direction prior to rotation).  Defaults to 1.0.",
				false, false,
				std::bind( &RectangularRoom::set_dimensions, this, std::placeholders::_1 , std::placeholders::_2 )
			)
		);

		// Work functions:
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< std::string > >(
				"type_specific_details_string", "Get a string describing the subclass-specific details of this room.  This override writes "
				"'LENGTH: <length> WIDTH: <width>'.",
				true, false, true, false,
				"type_specific_details_string", "A string describing the subclass-specific details of this room.",
				std::bind( &RectangularRoom::type_specific_details_string, this )
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

/// @brief Configure this object from a line in a problem definition file.
/// @details This override expects a line of the format "RectangularRoom <xcoord>
/// <ycoord> <angle_degrees> <radius> <seat_count> <optional local seat indices to omit>".
void
RectangularRoom::configure_from_input_line(
	std::string const & file_line
) {
	std::string const linetrimmed( masala::base::utility::string::trim( file_line ) );
	std::istringstream ss( linetrimmed );
	std::string firststring;
	std::lock_guard< std::mutex > lock( mutex() );
	masala::base::Real x, y, angle_degrees, length, width;
	ss >> firststring >> x >> y >> angle_degrees >> length >> width;
	CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Could not parse line \"" + linetrimmed + "\"." );
	
	CHECK_OR_THROW_FOR_CLASS( ss.eof(), "configure_from_input_line", "Extra input in line \"" + linetrimmed + "\"." );
	CHECK_OR_THROW_FOR_CLASS( firststring == "RectangularRoom", "configure_from_input_line", "Expected RectangularRoom configuration line to start with \"RectangularRoom\", but got \"" + linetrimmed + "\"." );
	CHECK_OR_THROW_FOR_CLASS( length > 0, "configure_from_input_line", "Error parsing line \"" + linetrimmed + "\": expected a length greater than zero." );
	CHECK_OR_THROW_FOR_CLASS( width > 0, "configure_from_input_line", "Error parsing line \"" + linetrimmed + "\": expected a width greater than zero." );
	protected_set_angle( angle_degrees ); // Converts to range [0, 360).
	protected_set_coordinates( x, y );
	protected_set_dimensions( length, width );
}

/// @brief Set the dimensions of the room.
/// @param length_in The length of the table, in meters (in the north-south direction prior to rotation).  Defaults to 1.0.
/// @param width_in The width of the table, in meters (in the east-west direction prior to rotation).  Defaults to 1.0.
void
RectangularRoom::set_dimensions(
	masala::base::Real const length_in,
	masala::base::Real const width_in
) {
	std::lock_guard< std::mutex > lock( mutex() );
	protected_set_dimensions( length_in, width_in );
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


/// @brief Get a string describing the subclass-specific details of this room.  This override writes
/// out "LENGTH: <length> WIDTH: <width>".
std::string
RectangularRoom::type_specific_details_string() const {
	std::lock_guard< std::mutex > lock( mutex() );
	std::ostringstream ss;
	ss << std::setprecision(6);
	ss << "LENGTH:\t" << length_ << "\tWIDTH:\t" << width_;
	return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
RectangularRoom::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
RectangularRoom::protected_assign( SeatingElementBase const & src ) {
	RectangularRoom const * const src_ptr_cast( dynamic_cast< RectangularRoom const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a RectangularRoom object."
	);

	// TODO TODO TODO
	length_ = src_ptr_cast->length_;
	width_ = src_ptr_cast->width_;

	Parent::protected_assign( src );
}

/// @brief Set the dimensions, with no mutex lock.
void
RectangularRoom::protected_set_dimensions(
	masala::base::Real const length_in,
	masala::base::Real const width_in
) {
	CHECK_OR_THROW_FOR_CLASS( length_in > 0, "protected_set_dimensions", "Expected the length of the room to be a positive value." );
	CHECK_OR_THROW_FOR_CLASS( width_in > 0, "protected_set_dimensions", "Expected the width of the room to be a positive value." );
	length_ = length_in;
	width_ = width_in;
}

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins