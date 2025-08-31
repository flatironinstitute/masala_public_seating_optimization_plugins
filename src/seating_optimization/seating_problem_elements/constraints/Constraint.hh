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

/// @file src/seating_optimization/seating_problem_elements/constraints/Constraint.hh
/// @brief Headers for a Constraint.
/// @details The Constraint class is the base class for constraints, which are elements of a seating problem.  They can
/// be things like, "Guests A and B should be seated next to one another," or "Guest C should be near the front of the room", etc.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_constraints_Constraint_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_constraints_Constraint_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/constraints/Constraint.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>

// Numeric API headers:
#include <numeric_api/base_classes/optimization/cost_function_network/PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem.fwd.hh>

// Base headers:
#include <base/types.hh>
#include <vector>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace constraints {

/// @brief A Constraint.
/// @details The Constraint class is the base class for constraints, which are elements of a seating problem.  They can
/// be things like, "Guests A and B should be seated next to one another," or "Guest C should be near the front of the room", etc.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class Constraint : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBase {

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
	Constraint() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	Constraint( Constraint const & src );

	/// @brief Destructor.
	~Constraint() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	ConstraintSP
	deep_clone() const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Get the category or categories for this plugin class.  Default for all
	/// optimization problems; may be overridden by derived classes.
	/// @returns { { "SeatingProblem", "SeatingProblemElement", "Constraint" } }
	/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
	/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
	/// in more than one hierarchical category (in which case there would be more than one
	/// entry in the outer vector), but must be in at least one.  The first one is used as
	/// the primary key.
	std::vector< std::vector< std::string > >
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem", "seating_problem_element", "constraint" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "Constraint".
	std::string
	class_name() const override;

	/// @brief Get the namespace for this class.
	/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints".
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
	virtual void configure_from_input_line( std::string const & input_line );

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Modify a pairwise precomputed cost function network optimization problem to add
	/// the constraint to it.
	/// @details Base class implementation throws.  Must be overridden by derived classes.
	virtual
	void
	add_constraint_to_cfn_problem(
		masala::numeric_api::base_classes::optimization::cost_function_network::PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem & cfn_problem,
		masala::base::Real const global_strength_multiplier
	) const;

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

}; // class Constraint

} // namespace constraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_constraints_Constraint_hh