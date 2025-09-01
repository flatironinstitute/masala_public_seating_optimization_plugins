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

/// @file src/seating_optimization/seating_problem_elements/Table.hh
/// @brief Headers for a Table.
/// @details A Table is an object around which a bunch of Seat objects are arranged, at which several people may sit.
/// This is intended to be a non-instantiable base class for concrete derived classes defining various table shapes.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Table_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Table_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/Table.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Seat.fwd.hh>

// Base headers:
#include <base/types.hh>
#include <vector>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

/// @brief A Table.
/// @details A Table is an object around which a bunch of Seat objects are arranged, at which several people may sit.
/// This is intended to be a non-instantiable base class for concrete derived classes defining various table shapes.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class Table : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBase {

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
	Table() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	Table( Table const & src );

	/// @brief Destructor.
	~Table() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	TableSP
	deep_clone() const;

public:

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
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem", "seating_problem_element", "table" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "Table".
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

	/// @brief Get the x-coordinate of the centre of the table.
	Real x() const;

	/// @brief Get the y-coordinate of the centre of the table.
	Real y() const;

	/// @brief Get the orientation of the table.
	/// @details A table has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).
	Real angle() const;

	/// @brief Get the number of seats that this table has.
	Size num_seats() const;

	/// @brief Access a particular seat, by local index (staring at 0 with the first seat around this table).  Throws if seat out of range.
	SeatCSP seat( Size const seat_index ) const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Set the coordinates of the table's centre.  A table has coordinates in R^2 (x and y).
	void set_coordinates( Real const x_in, Real const y_in );

	/// @brief Set the table's angle.
	/// @details A table has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).	
	void
	set_angle( Real const angle_degrees_in );

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Get a list of seats that are next to one another at this table.
	/// @details Base class implementation throws.  Must be implemented by derived classes.
	virtual
	std::vector< std::pair< SeatCSP, SeatCSP > >
	get_adjacent_seats() const;

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

	/// @brief Allow derived classes to access the seats vector.  This is expected to occur under mutex lock, but
	/// this function does no mutex-locking.
	std::vector< SeatSP > & protected_seats();

	/// @brief Allow derived classes to access the seats vector (const access).  This is expected to occur under mutex lock, but
	/// this function does no mutex-locking.
	std::vector< SeatCSP > protected_seats_const() const;

	/// @brief Update the coordinates of seats on a change of table coordinates or dimensions.
	/// @details Base class throws.  Derived classes should override this.
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	virtual void protected_update_seat_coordinates();

	/// @brief Allow derived classes to access the x-coordinate of the centre of the table.
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	inline Real protected_x() const { return x_; }

	/// @brief Allow derived classes to access the y-coordinate of the centre of the table.
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	inline Real protected_y() const { return y_; }

	/// @brief Allow derived classes to access the table's orientation.
	/// @details A table has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	inline Real protected_angle_degrees() const { return angle_degrees_; }

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

	/// @brief A set of seats associated with this table.
	std::vector< SeatSP > seats_;

}; // class Table

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Table_hh