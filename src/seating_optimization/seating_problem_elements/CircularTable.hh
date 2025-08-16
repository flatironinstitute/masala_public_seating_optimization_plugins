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

/// @file src/seating_optimization/seating_problem_elements/CircularTable.hh
/// @brief Headers for a CircularTable.
/// @details As the name suggests, a circular table is a table with a circular shape and chairs arranged around it.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_CircularTable_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_CircularTable_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/CircularTable.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/Table.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Seat.fwd.hh>

// Base headers:
#include <base/types.hh>
#include <vector>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

/// @brief A CircularTable.
/// @details As the name suggests, a circular table is a table with a circular shape and chairs arranged around it.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class CircularTable : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::Table {

	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::Table Parent;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableSP ParentSP;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP ParentCSP;

	typedef masala::base::Real Real;
	typedef masala::base::Size Size;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	CircularTable() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	CircularTable( CircularTable const & src );

	/// @brief Destructor.
	~CircularTable() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	CircularTableSP
	deep_clone() const;

public:

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
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem", "seating_problem_element", "table", "circular" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "CircularTable".
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

	/// @brief Set the radius of the table.
	/// @param radius_in The radius of the table, in meters.  Note that this is the radius from
	/// the centre at which seat centres are found, not the radius of the tabletop.  Defaults to 1.0.
	void set_radius( Real const radius_in );

	/// @brief Set the number of seats evenly spaced around the table.  Clears any existing seats.
	/// @param seat_count_in The number of seats to space around the table evenly.
	/// @param omitted_seats An optional set of seat indices (zero-based) to omit.  This can be useful if, for instance,
	/// a table is against a wall, or one seat would be too close to a pillar, or whatnot.  Leave this as an empty vector
	/// to have seats evenly spaced all the way around the table.
	void set_seat_count( Size const seat_count_in, std::vector< Size > const & omitted_seats );

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

	/// @brief Update the coordinates of seats on a change of table coordinates or dimensions.
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	void protected_update_seat_coordinates() override;

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
////////////////////////////////////////////////////////////////////////////////

	/// @brief The radius of the table, in meters.  Note that this is the distance from the centre at which seat centres are
	/// found, not the radius of the tabletop.  Defaults to 1.0.
	Real radius_ = 1.0;

}; // class CircularTable

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_CircularTable_hh