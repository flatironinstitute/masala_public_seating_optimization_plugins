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

/// @file src/seating_optimization/seating_problem_elements/constraints/Constraint.cc
/// @brief Implementations for a Constraint.
/// @details The Constraint class is the base class for constraints, which are elements of a seating problem.  They can
/// be things like, "Guests A and B should be seated next to one another," or "Guest C should be near the front of the room", etc.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/constraints/Constraint.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem/SeatingProblem.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ThreeInput.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace constraints {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
Constraint::Constraint( Constraint const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	Constraint::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
Constraint::clone() const {
	return masala::make_shared< Constraint >( *this );
}

/// @brief Make a fully independent copy of this object.
ConstraintSP
Constraint::deep_clone() const {
	ConstraintSP new_guest( std::static_pointer_cast< Constraint >( this->clone() ) );
	new_guest->make_independent();
	return new_guest;
}

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
Constraint::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Constraint" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "constraint" }
std::vector< std::string >
Constraint::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"guest",
	};
}

/// @brief Get the name of this class.
/// @returns "Constraint".
std::string
Constraint::class_name() const {
	return "Constraint";
}

/// @brief Get the namespace for this class.
/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints".
std::string
Constraint::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the Constraint class.
masala::base::api::MasalaObjectAPIDefinitionCWP
Constraint::get_api_definition() {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::api::work_function;
	using masala::base::Real;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex() );

	if( api_definition() == nullptr ) {

		MasalaObjectAPIDefinitionSP api_def(
			masala::make_shared< MasalaObjectAPIDefinition >(
				*this,
				"The Constraint class is the base class for constraints, which are elements of a seating problem.  They can "
				"be things like, 'Guests A and B should be seated next to one another,' or 'Guest C should be near the front of the "
				"room', etc.  This is a base class that is not intended to be instantiated outside of the API system.",
				false, true
			)
		);
		ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( Constraint, api_def );

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line", "Configure this object from a line in an input file.  "
				"Base class implementation throws.  Must be overridden by derived classes.",
				"input_line", "The line from which we are configuring this object.  Syntax depends on "
				"derived class.  Must start with an identifier for the constraint type.",
				true, false,
				std::bind( &Constraint::configure_from_input_line, this, std::placeholders::_1 )
			)
		);

		// Work functions:
		api_def->add_work_function(
			masala::make_shared<
				MasalaObjectAPIWorkFunctionDefinition_ThreeInput<
					void,
					seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const &,
					masala::numeric::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem &,
					masala::base::Real const
				>
			>(
				"add_constraint_to_cfn_problem", "Modify a pairwise precomputed cost function network optimization problem to add "
				"the constraint to it.  Base class implementation throws.  Must be overridden by derived classes.",
				true, false, true, false,
				"seating_problem", "The object describing the seats, tables, guests, and constraints.",
				"cfn_problem", "The cost function network optimizaton problem, modified by this operation.",
				"global_strength_multiplier", "A global multiplier for the strength of this constraint.",
				"void", "This function returns nothing.",
				std::bind( &Constraint::add_constraint_to_cfn_problem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 )
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

/// @brief Configure this object from a line in an input file.
/// @details Base class implementation throws.  Must be overridden by derived classes.
/*virtual*/
void
Constraint::configure_from_input_line(
	std::string const & //input_line
) {
	MASALA_THROW( class_namespace() + "::" + class_name(), "configure_from_input_line", "This class must override this function." );
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


/// @brief Modify a pairwise precomputed cost function network optimization problem to add
/// the constraint to it.
/// @details Base class implementation throws.  Must be overridden by derived classes.
/*virtual*/
void
Constraint::add_constraint_to_cfn_problem(
	seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & ,//seating_problem,
	masala::numeric::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem & ,//cfn_problem,
	masala::base::Real const //global_strength_multiplier
) const {
	MASALA_THROW( class_namespace() + "::" + class_name(), "add_constraint_to_cfn_problem", "This class must override this function." );
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Constraint::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Constraint::protected_assign( SeatingElementBase const & src ) {
	Constraint const * const src_ptr_cast( dynamic_cast< Constraint const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a Constraint object."
	);

	// TODO TODO TODO

	Parent::protected_assign( src );
}

} // namespace constraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
