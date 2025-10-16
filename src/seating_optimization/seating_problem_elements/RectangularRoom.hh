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

/// @file src/seating_optimization/seating_problem_elements/RectangularRoom.hh
/// @brief Headers for a RectangularRoom.
/// @details As the name suggests, a rectangular room is a room with a rectangular shape.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_RectangularRoom_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_RectangularRoom_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/RectangularRoom.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/Room.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Seat.fwd.hh>

// Base headers:
#include <base/types.hh>
#include <vector>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

/// @brief A RectangularRoom.
/// @details As the name suggests, a rectangular room is a room with a rectangular shape.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class RectangularRoom : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::Room {

	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::Room Parent;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::RoomSP ParentSP;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::RoomCSP ParentCSP;

	typedef masala::base::Real Real;
	typedef masala::base::Size Size;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	RectangularRoom() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	RectangularRoom( RectangularRoom const & src );

	/// @brief Destructor.
	~RectangularRoom() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	RectangularRoomSP
	deep_clone() const;

public:

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
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem", "seating_problem_element", "room", "rectangular" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "RectangularRoom".
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
	/// @details This override expects a line of the format "RectangularRoom <xcoord>
	/// <ycoord> <angle_degrees> <length> <width>".
	void configure_from_input_line( std::string const & file_line ) override;

	/// @brief Set the dimensions of the room.
	/// @param length_in The length of the table, in meters (in the north-south direction prior to rotation).  Defaults to 1.0.
	/// @param width_in The width of the table, in meters (in the east-west direction prior to rotation).  Defaults to 1.0.
	void set_dimensions( masala::base::Real const length_in, masala::base::Real const width_in );

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

	/// @brief Set the dimensions, with no mutex lock.
	void
	protected_set_dimensions(
		masala::base::Real const length_in,
		masala::base::Real const width_in
	);

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
////////////////////////////////////////////////////////////////////////////////

	/// @brief The length of the room (in the north-south direction prior to rotation).
	masala::base::Real length_ = 1.0;

	/// @brief The width of the room (in the north-south direction prior to rotation).
	masala::base::Real width_ = 1.0;

}; // class RectangularRoom

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_RectangularRoom_hh