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

/// @file src/seating_optimization/seating_problem_elements/CircularTable.cc
/// @brief Implementations for a CircularTable.
/// @details As the name suggests, a circular table is a table with a circular shape and chairs arranged around it.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/CircularTable.hh>

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
CircularTable::CircularTable( CircularTable const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	CircularTable::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
CircularTable::clone() const {
	return masala::make_shared< CircularTable >( *this );
}

/// @brief Make a fully independent copy of this object.
CircularTableSP
CircularTable::deep_clone() const {
	CircularTableSP new_table( std::static_pointer_cast< CircularTable >( this->clone() ) );
	new_table->make_independent();
	return new_table;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem", "SeatingProblemElement", "Table", "CircularTable" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
CircularTable::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Table", "CircularTable" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "table", "circular" }
std::vector< std::string >
CircularTable::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"table",
		"circular"
	};
}

/// @brief Get the name of this class.
/// @returns "CircularTable".
std::string
CircularTable::class_name() const {
	return "CircularTable";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
CircularTable::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the CircularTable class.
masala::base::api::MasalaObjectAPIDefinitionCWP
CircularTable::get_api_definition() {
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
				"The CircularTable class stores all information associated with a circular table around which seats are arranged.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( CircularTable, api_def );

		// Work functions:
		// api_def->add_work_function(
		// 	masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< std::vector< std::pair< SeatCSP, SeatCSP > > > >(
		// 		"get_adjacent_seats", "Get a list of seats that are next to one another at this table.  Base class implementation "
		// 		"throws.  Must be implemented by derived classes.",
		// 		false, false, false, true,
		// 		"adjacent_seats", "A vector of pairs of const shared pointers to the seats at this table that are adjacent to each other.",
		// 		std::bind( &CircularTable::get_adjacent_seats, this )
		// 	)
		// );

		// Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"x",
				"Get the x-coordinate of the table's centre.",
				"x", "The x-coordinate of the tables's centre, in meters.",
				false, false,
				std::bind( &CircularTable::x, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"y",
				"Get the y-coordinate of the table's centre.",
				"y", "The y-coordinate of the tables's centre, in meters.",
				false, false,
				std::bind( &CircularTable::y, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Real > >(
				"angle",
				"Get the orientation of the table.  A table has an orientation, defined as the clockwise angle, in degrees, from facing "
				"north (the (0,1) direction in x-y space).  Note that for a circular table, the angle defines the rotation of the chairs "
				"around the centre of the table.",
				"angle_degrees", "The table's orientation, in degrees, defined as the clockwise angle from facing north (the (0,1) "
				"direction in x-y space).",
				false, false,
				std::bind( &CircularTable::angle, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< Size > >(
				"num_seats",
				"Get the number of seats that this table has.",
				"num_seats", "The number of seats that this table has.",
				false, false,
				std::bind( &CircularTable::num_seats, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_OneInput< SeatCSP, Size > >(
				"seat",
				"Access a particular seat, by local index (staring at 0 with the first seat around this table).  "
				"Throws if seat out of range.",
				"seat_index", "The local, zero-based index of this seat.",
				"seat", "A shared pointer to the Seat object.",
				false, false,
				std::bind( &CircularTable::seat, this, std::placeholders::_1 )
			)
		);

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line",
				"Configure this object from a line in a problem definition file.  This implementation expects "
				"a line of the format 'Seat <xcoord> <ycoord> <angle_degrees> <radius> <seat_count> <optional local seat indices to omit>'.",
				"file_line", "A line from a configuration file.  Should start with the class name.",
				false, true,
				std::bind( &CircularTable::configure_from_input_line, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Real const, Real const > >(
				"set_coordinates",
				"Set the coordinates of the centre of the table.  A table has coordinates in R^2 (x and y).",
				"x_in", "The x coordinate, in meters.",
				"y_in", "The y coordinate, in meters.",
				false, false,
				std::bind( &CircularTable::set_coordinates, this, std::placeholders::_1, std::placeholders::_1 )
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
				std::bind( &CircularTable::set_angle, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< Real const > >(
				"set_radius",
				"Set the radius of the table (the distance from the centre at which seat centres are found), in meters.",
				"radius_in", "The input radius of the table, in meters.  Note that this is the radius from the centre at "
				"which seat centres are found, not the radius of the tabletop.  Defaults to 1.0.",
				false, false,
				std::bind( &CircularTable::set_radius, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Size const, std::vector< Size > const & > >(
				"set_seat_count",
				"Set the number of seats evenly spaced around the table.  Clears any existing seats.",
				"seat_count_in", "The number of seats to space around the table evenly.",
				"omitted_seats", "An optional set of seat indices (zero-based) to omit.  This can be useful if, for instance, "
				"a table is against a wall, or one seat would be too close to a pillar, or whatnot.  Leave this as an empty vector "
				"to have seats evenly spaced all the way around the table.",
				false, false,
				std::bind( &CircularTable::set_seat_count, this, std::placeholders::_1, std::placeholders::_2 )
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
/// @details This override expects a line of the format "CircularTable <xcoord>
/// <ycoord> <angle_degrees> <radius> <seat_count> <optional local seat indices to omit>".
void
CircularTable::configure_from_input_line(
	std::string const & file_line
) {
	std::string const linetrimmed( masala::base::utility::string::trim( file_line ) );
	std::istringstream ss( linetrimmed );
	std::string firststring;
	std::lock_guard< std::mutex > lock( mutex() );
	masala::base::Real x, y, angle_degrees, radius;
	masala::base::Size seatcount;
	ss >> firststring >> x >> y >> angle_degrees >> radius >> seatcount;
	CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Could not parse line \"" + linetrimmed + "\"." );
	std::vector< masala::base::Size > seats_to_omit;
	while( !ss.eof() ) {
		masala::base::Size seatindex;
		ss >> seatindex;
		CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Could not parse additional input in line \"" + linetrimmed + "\".  Expected zero-based local indices of seats to omit." );
		CHECK_OR_THROW_FOR_CLASS( seatindex < seatcount, "configure_from_input_line", "Error parsing line \"" + linetrimmed + "\": bad omitted seat index " + std::to_string(seatindex) + "."  );
		CHECK_OR_THROW_FOR_CLASS( !masala::base::utility::container::has_value(seats_to_omit, seatindex), "configure_from_input_line", "Error parsing line \"" + linetrimmed + "\": omitted seat " + std::to_string(seatindex) + " appears more than once." );
		seats_to_omit.push_back( seatindex );
	}
	CHECK_OR_THROW_FOR_CLASS( ss.eof(), "configure_from_input_line", "Extra input in line \"" + linetrimmed + "\"." );
	CHECK_OR_THROW_FOR_CLASS( firststring == "CircularTable", "configure_from_input_line", "Expected CircularTable configuration line to start with \"CircularTable\", but got \"" + linetrimmed + "\"." );
	CHECK_OR_THROW_FOR_CLASS( radius > 0, "configure_from_input_line", "Error parsing line \"" + linetrimmed + "\": expected a radius greater than zero." );
	protected_set_angle( angle_degrees ); // Converts to range [0, 360).
	protected_set_coordinates( x, y );
	radius_ = radius;
	protected_set_seat_count( seatcount, seats_to_omit );
}

/// @brief Set the radius of the table.
/// @param radius_in The radius of the table, in meters.  Note that this is the radius from
/// the centre at which seat centres are found, not the radius of the physical table.
void
CircularTable::set_radius(
	Real const radius_in
) {
	CHECK_OR_THROW_FOR_CLASS( radius_in > 0.0, "set_radius", "The table radius must be positive." );
	std::lock_guard< std::mutex > lock( mutex() );
	radius_ = radius_in;
	protected_update_seat_coordinates();
}

/// @brief Set the number of seats evenly spaced around the table.  Clears any existing seats.
/// @param seat_count_in The number of seats to space around the table evenly.
/// @param omitted_seats An optional set of seat indices (zero-based) to omit.  This can be useful if, for instance,
/// a table is against a wall, or one seat would be too close to a pillar, or whatnot.  Leave this as an empty vector
/// to have seats evenly spaced all the way around the table.
void
CircularTable::set_seat_count(
	Size const seat_count_in,
	std::vector< Size > const & omitted_seats
) {
	std::lock_guard< std::mutex > lock( mutex() );
	protected_set_seat_count( seat_count_in, omitted_seats );
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a list of seats that are next to one another at this table.
/// @details Base class implementation throws.  Must be implemented by derived classes.
std::vector< std::pair< SeatCSP, SeatCSP > >
CircularTable::get_adjacent_seats() const {
	std::lock_guard< std::mutex > lock( mutex() );

	std::vector< SeatCSP > const seats( protected_seats_const() );
	if(seats.size() < 2) {
		return std::vector< std::pair< SeatCSP, SeatCSP > >{};
	}
	std::vector< std::pair< SeatCSP, SeatCSP > > outvec( seats.size() );
	outvec[0] = std::make_pair( seats[0], seats[seats.size()-1] );
	for( Size i(1); i<seats.size(); ++i ) {
		outvec[i] = std::make_pair( seats[i-1], seats[i] );
	}
	return outvec;
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
CircularTable::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
CircularTable::protected_assign( SeatingElementBase const & src ) {
	CircularTable const * const src_ptr_cast( dynamic_cast< CircularTable const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a CircularTable object."
	);

	// TODO TODO TODO
	radius_ = src_ptr_cast->radius_;

	Parent::protected_assign( src );
}

/// @brief Update the coordinates of seats on a change of table coordinates or dimensions.
void
CircularTable::protected_update_seat_coordinates() {
	std::vector< SeatSP > & seats( protected_seats() );
	if( seats.empty() ) { return; }

	Real const angle_increment( 360.0 / static_cast< Real >( seats.size() ) );
	for( Size i(0); i<seats.size(); ++i ) {
		SeatSP & seat( seats[i] );
		if( seat != nullptr ) {
			Real const curangle( protected_angle_degrees() + angle_increment * i );
			seat->set_angle( curangle );
			Real const curangle_radians( curangle / 180.0 * MASALA_PI );
			seat->set_coordinates(
				std::sin( curangle_radians ) * radius_,
				std::cos( curangle_radians ) * radius_
			);
		}
	}
}

/// @brief Set the number of seats evenly spaced around the table.  Clears any existing seats.  Performs no mutex locking.
/// @param seat_count_in The number of seats to space around the table evenly.
/// @param omitted_seats An optional set of seat indices (zero-based) to omit.  This can be useful if, for instance,
/// a table is against a wall, or one seat would be too close to a pillar, or whatnot.  Leave this as an empty vector
/// to have seats evenly spaced all the way around the table.
void
CircularTable::protected_set_seat_count(
	masala::base::Size const seat_count_in,
	std::vector< masala::base::Size > const & omitted_seats
) {
	std::vector< SeatSP > & seats( protected_seats() );
	seats.clear();
	if( seat_count_in == 0 ) { return; }
	seats.resize( seat_count_in, nullptr );
	seats.shrink_to_fit();
	std::vector< bool > omitted_seat_bools( seat_count_in, false );
	for( Size const entry : omitted_seats ) {
		CHECK_OR_THROW_FOR_CLASS( entry < seat_count_in, "set_seat_count", "Cannot omit seat " + std::to_string( entry ) + " given a table with "
			"only " + std::to_string( seat_count_in ) + " seats."
		);
		omitted_seat_bools[entry] = true;
	}
	Size counter(0);
	for( SeatSP & seat : seats ) {
		if( !omitted_seat_bools[counter] ) {
			seat = masala::make_shared< Seat >();
		}
		++counter;
	}

	protected_update_seat_coordinates();
}

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins