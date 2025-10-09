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

/// @file src/seating_optimization/seating_problem_elements/Seat.hh
/// @brief Headers for a Seat.
/// @details A Seat is a place where a person can sit.  It's defined by a coordinate in R^2 and
/// a direction (an angle, measured clockwise from north).
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Seat_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Seat_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/Seat.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>

// Base headers:
#include <base/types.hh>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

/// @brief A Seat.
/// @details A Seat is a place where a person can sit.  It's defined by a coordinate in R^2 and
/// a direction (an angle, measured clockwise from north).
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class Seat : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBase {

	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBase Parent;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP ParentSP;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseCSP ParentCSP;

	typedef masala::base::Real Real;
	typedef masala::base::Size Size;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	Seat() = default;

	/// @brief Options constructor.
	Seat(
		Real const x_in,
		Real const y_in,
		Real const angle_in
	);

	/// @brief Copy constructor.  Explicit due to mutex.
	Seat( Seat const & src );

	/// @brief Destructor.
	~Seat() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	SeatSP
	deep_clone() const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Get the category or categories for this plugin class.  Default for all
	/// optimization problems; may be overridden by derived classes.
	/// @returns  { { "SeatingProblem", "SeatingProblemElement", "Seat" } }
	/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
	/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
	/// in more than one hierarchical category (in which case there would be more than one
	/// entry in the outer vector), but must be in at least one.  The first one is used as
	/// the primary key.
	std::vector< std::vector< std::string > >
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns  { "seating_problem", "seating_problem_element", "seat" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "Seat".
	std::string
	class_name() const override;

	/// @brief Get the namespace for this class.
	/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements".
	std::string
	class_namespace() const override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

    /// @brief Get a description of the API for the CostFunctionNetworkOptimizationSolution class.
    masala::base::api::MasalaObjectAPIDefinitionCWP
    get_api_definition() override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////


public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Configure this object from a line in a problem definition file.
	/// @details This override expects a line of the format "Seat <xcoord> <ycoord> <angle_degrees>".
	void configure_from_input_line( std::string const & file_line ) override;

	/// @brief Set the seat's coordinates.  A seat has coordinates in R^2 (x and y).
	void set_coordinates( Real const x_in, Real const y_in );

	/// @brief Set the seat's angle.
	/// @details A seat has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).	
	void set_angle( Real const angle_degrees_in );

	/// @brief Indicate that this seat is associated with a table, and has a local index at that table.
	void
	set_table_index_and_local_seat_index(
		masala::base::Size const table_index,
		masala::base::Size const local_seat_index_at_table
	);

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

protected:

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
	/// the parent class implementation.
	void protected_make_independent() override;

	/// @brief Assign src to this object.  Derived classes must override this, and the override must call
	/// the parent class implementation.
	void protected_assign( SeatingElementBase const & src ) override;

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
////////////////////////////////////////////////////////////////////////////////

	/// @brief A seat has an x-coordinate.
	Real x_ = 0.0;

	/// @brief A seat has a y-coordinate.
	Real y_ = 0.0;

	/// @brief A seat has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).
	Real angle_degrees_ = 0.0;

	/// @brief Is this seat associated with a table?
	bool at_a_table_ = false;

	/// @brief The index of the table with which this seat is associated.
	masala::base::Size table_index_ = 0;

	/// @brief This seat's local index at its table.
	masala::base::Size local_index_at_table_ = 0;


}; // class Seat

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Seat_hh