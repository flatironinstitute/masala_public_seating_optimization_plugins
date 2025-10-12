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

/// @file src/seating_optimization/seating_problem_elements/restraints/RestrictGuestToSeatsRestraint.hh
/// @brief Headers for a RestrictGuestToSeatsRestraint.
/// @details The RestrictGuestToTableRestraint class strictly limits a guest to a particular set of seats.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_restraints_RestrictGuestToSeatsRestraint_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_restraints_RestrictGuestToSeatsRestraint_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/restraints/RestrictGuestToSeatsRestraint.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/restraints/Restraint.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem/SeatingProblem.fwd.hh>

// Numeric headers:
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.fwd.hh>

// Base headers:
#include <base/types.hh>
#include <vector>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace restraints {

/// @brief A RestrictGuestToSeatsRestraint.
/// @details The RestrictGuestToTableRestraint class strictly limits a guest to a particular set of seats.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class RestrictGuestToSeatsRestraint : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::Restraint {

	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::Restraint Parent;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::RestraintSP ParentSP;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::RestraintCSP ParentCSP;

	typedef masala::base::Real Real;
	typedef masala::base::Size Size;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	RestrictGuestToSeatsRestraint() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	RestrictGuestToSeatsRestraint( RestrictGuestToSeatsRestraint const & src );

	/// @brief Destructor.
	~RestrictGuestToSeatsRestraint() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	RestrictGuestToSeatsRestraintSP
	deep_clone() const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Get the category or categories for this plugin class.  Default for all
	/// optimization problems; may be overridden by derived classes.
	/// @returns { { "SeatingProblem", "SeatingProblemElement", "Restraint" } }
	/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
	/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
	/// in more than one hierarchical category (in which case there would be more than one
	/// entry in the outer vector), but must be in at least one.  The first one is used as
	/// the primary key.
	std::vector< std::vector< std::string > >
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem", "seating_problem_element", "restraint" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "RestrictGuestToSeatsRestraint".
	std::string
	class_name() const override;

	/// @brief Get the namespace for this class.
	/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints".
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

	/// @brief Configure this object from a line in an input file.
	/// @details Base class implementation throws.  Must be overridden by derived classes.
	void configure_from_input_line( std::string const & input_line ) override;

	/// @brief Set the guest UID.
	void set_guest_uid( std::string const & setting );

	/// @brief Add a seat to the set to which we will restrict this guest.
	void add_seat( masala::base::Size const setting );

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Limit the allowed seating choices for the guests.
	/// @details Base class implementation throws.  Must be overridden by derived classes.  The allowed_seating_choices_by_guest
	/// vector is a vector with one entry per guest, where each entry is a Boolean vector with one entry per seat.  RestrictGuestToSeatsRestraints can
	/// set some subset of these Booleans to false.
	void
	restrain_seating_choices(
		seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & seating_problem,
		std::vector< std::vector< bool > > & allowed_seating_choices_by_guest
	) const override;

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

	/// @brief The Guest UID.
	std::string guest_uid_;

	/// @brief The table index.
	std::vector< masala::base::Size > seats_;

}; // class RestrictGuestToSeatsRestraint

} // namespace restraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_restraints_RestrictGuestToSeatsRestraint_hh