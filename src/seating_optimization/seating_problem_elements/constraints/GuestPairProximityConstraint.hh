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

/// @file src/seating_optimization/seating_problem_elements/constraints/GuestPairProximityConstraint.hh
/// @brief Headers for a GuestPairProximityConstraint.
/// @details The GuestPairProximityConstraint class is used to indicate that two guests should be close together or should not be close together.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_constraints_GuestPairProximityConstraint_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_constraints_GuestPairProximityConstraint_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/constraints/GuestPairProximityConstraint.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/constraints/Constraint.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem/SeatingProblem.fwd.hh>

// Numeric headers:
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.fwd.hh>

// Base headers:
#include <base/types.hh>
#include <vector>
#include <string>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace constraints {

////////////////////////////////////////////////////////////////////////////////
// ProximityFalloffMode ENUM CLASS
////////////////////////////////////////////////////////////////////////////////

/// @brief A class for the type of falloff.
/// @details If you add to this, update falloff_mode_string_from_enum() and GuestPairProximityConstraint::configure_from_input_line().
enum class ProximityFalloffMode {
	INVALID_MODE = 0, // Keep this first
	// Add modes here:
	GAUSSIAN, // Keep this second-to-last
	N_FALLOFF_MODES = GAUSSIAN // Keep this last.
};

/// @brief Given the falloff mode as an enum, get the string.
std::string falloff_mode_string_from_enum( ProximityFalloffMode const mode_enum);

/// @brief Given the falloff mode as a string, get the enum.
/// @details Returns ProximityFalloffMode::INVALID_MODE if the string can't be interpreted as a proximity mode.
ProximityFalloffMode falloff_mode_enum_from_string( std::string const & modestring );

////////////////////////////////////////////////////////////////////////////////
// GuestPairProximityConstraint CLASS
////////////////////////////////////////////////////////////////////////////////

/// @brief A GuestPairProximityConstraint.
/// @details The GuestPairProximityConstraint class is used to indicate that two guests should be close together or should not be close together.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class GuestPairProximityConstraint : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::Constraint {

	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::Constraint Parent;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::ConstraintSP ParentSP;
	typedef seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::ConstraintCSP ParentCSP;

	typedef masala::base::Real Real;
	typedef masala::base::Size Size;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	GuestPairProximityConstraint() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	GuestPairProximityConstraint( GuestPairProximityConstraint const & src );

	/// @brief Destructor.
	~GuestPairProximityConstraint() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	GuestPairProximityConstraintSP
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
	/// @returns "GuestPairProximityConstraint".
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
	/// @details Base class implementation throws.  Must be overridden by derived classes.  This version expects
	/// a line of the form "GuestPairProximityConstraint <guest1_uid> <guest2_uid> <constraint_strength_at_one_unit> <falloff_type> <falloff_options...>".
	void configure_from_input_line( std::string const & input_line ) override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Modify a pairwise precomputed cost function network optimization problem to add
	/// the constraint to it.
	/// @details Base class implementation throws.  Must be overridden by derived classes.
	void
	add_constraint_to_cfn_problem(
		seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & seating_problem,
		std::vector< std::vector< bool > > const & guest_to_allowed_seats,
		std::vector< std::map< masala::base::Size, masala::base::Size > > const & guest_to_seat_index_to_guest_choice,
		masala::numeric::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem & cfn_problem,
		masala::base::Real const global_strength_multiplier
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

	/// @brief First guest unique identifier.
	std::string first_guest_uid_;

	/// @brief Second guest unique identifier.
	std::string second_guest_uid_;

	/// @brief Constraint strength at a distance of one unit.  Defaults to 0.  Positive values indicate a penalty for two guests being side-by-side;
	/// negative values indicate a bonus.
	masala::base::Real constraint_strength_at_one_unit_ = 0.0;

	/// @brief The type of falloff.
	ProximityFalloffMode falloff_mode_ = ProximityFalloffMode::GAUSSIAN;

}; // class GuestPairProximityConstraint

} // namespace constraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_constraints_GuestPairProximityConstraint_hh