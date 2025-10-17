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

/// @file src/seating_optimization/seating_problem_elements/Room.hh
/// @brief Headers for a Room.
/// @details A Room is an object in which all the tables are arranged.  It is mainly used for visualization.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Room_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Room_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/Room.fwd.hh>

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

/// @brief A Room.
/// @details A Room is an object around which a bunch of Seat objects are arranged, at which several people may sit.
/// This is intended to be a non-instantiable base class for concrete derived classes defining various table shapes.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class Room : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBase {

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
	Room() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	Room( Room const & src );

	/// @brief Destructor.
	~Room() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	RoomSP
	deep_clone() const;

public:

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
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem", "seating_problem_element", "room" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "Room".
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

	/// @brief Get the x-coordinate of the centre of the room.
	Real x() const;

	/// @brief Get the y-coordinate of the centre of the room.
	Real y() const;

	/// @brief Get the orientation of the room.
	/// @details A room has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).
	Real angle() const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Set the coordinates of the room's centre.  A room has coordinates in R^2 (x and y).
	void set_coordinates( Real const x_in, Real const y_in );

	/// @brief Set the room's angle.
	/// @details A room has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).	
	void
	set_angle( Real const angle_degrees_in );

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Get a string describing the subclass-specific details of this room.  Base class
	/// implementation doesn't do anything; must be implemented by derived classes.
	virtual std::string type_specific_details_string() const;

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

	/// @brief Allow derived classes to access the x-coordinate of the centre of the room.
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	inline Real protected_x() const { return x_; }

	/// @brief Allow derived classes to access the y-coordinate of the centre of the room.
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	inline Real protected_y() const { return y_; }

	/// @brief Allow derived classes to access the room's orientation.
	/// @details A room has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).
	/// @note Performs no mutex-locking.  Should only be called in a mutex-locked context.
	inline Real protected_angle_degrees() const { return angle_degrees_; }

	/// @brief Allow derived classes to set coordinates.  Performs no mutex-locking; should be called from
	/// a mutex-locked context only.
	void protected_set_coordinates( masala::base::Real const x_in, masala::base::Real const y_in );

	/// @brief Allow derived classes to set the angle.  Performs no mutex-locking; should be called from
	/// a mutex-locked context only.  Angle is in degrees.
	void protected_set_angle( masala::base::Real const angle_degrees_in );

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
////////////////////////////////////////////////////////////////////////////////

	/// @brief A room has an x-coordinate.
	Real x_ = 0.0;

	/// @brief A room has a y-coordinate.
	Real y_ = 0.0;

	/// @brief A room has an orientation, defined as the clockwise angle, in degrees, from facing
	/// north (the (0,1) direction in x-y space).
	Real angle_degrees_ = 0.0;

}; // class Room

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_Room_hh