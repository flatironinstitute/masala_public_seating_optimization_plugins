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

/// @file src/seating_optimization/seating_problem_elements/restraints/Restraint.hh
/// @brief Headers for a Restraint.
/// @details The Restraint class is the base class for restraints, which are elements of a seating problem.  They stricly limit the
/// seating choices for a particular guest, unlike Constraints, which only impose a penalty or a bonus for a particular seating choice.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_restraints_Restraint_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_restraints_Restraint_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/restraints/Restraint.fwd.hh>

// Parent header:
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>

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

/// @brief A Restraint.
/// @details The Restraint class is the base class for restraints, which are elements of a seating problem.  They stricly limit the
/// seating choices for a particular guest, unlike Constraints, which only impose a penalty or a bonus for a particular seating choice.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class Restraint : public seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBase {

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
	Restraint() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	Restraint( Restraint const & src );

	/// @brief Destructor.
	~Restraint() override = default;

	/// @brief Make a copy of this object.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const override;

	/// @brief Make a fully independent copy of this object.
	RestraintSP
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
	/// @returns "Restraint".
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

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Limit the allowed seating choices for the guests.
	/// @details Base class implementation throws.  Must be overridden by derived classes.  The allowed_seating_choices_by_guest
	/// vector is a vector with one entry per guest, where each entry is a Boolean vector with one entry per seat.  Restraints can
	/// set some subset of these Booleans to false.
	virtual
	void
	restrain_seating_choices(
		seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & seating_problem,
		std::vector< std::vector< bool > > & allowed_seating_choices_by_guest
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

}; // class Restraint

} // namespace restraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_restraints_Restraint_hh