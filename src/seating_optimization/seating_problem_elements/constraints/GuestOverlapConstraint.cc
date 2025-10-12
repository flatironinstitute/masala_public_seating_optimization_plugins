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

/// @file src/seating_optimization/seating_problem_elements/constraints/GuestOverlapConstraint.cc
/// @brief Implementations for a GuestOverlapConstraint.
/// @details The GuestOverlapConstraint class is used to indicate that two guests cannot be assigned to the same seat.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/constraints/GuestOverlapConstraint.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem/SeatingProblem.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.hh>
#include <numeric_api/base_classes/optimization/cost_function_network/PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem.hh>

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
GuestOverlapConstraint::GuestOverlapConstraint( GuestOverlapConstraint const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	GuestOverlapConstraint::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
GuestOverlapConstraint::clone() const {
	return masala::make_shared< GuestOverlapConstraint >( *this );
}

/// @brief Make a fully independent copy of this object.
GuestOverlapConstraintSP
GuestOverlapConstraint::deep_clone() const {
	GuestOverlapConstraintSP new_guest( std::static_pointer_cast< GuestOverlapConstraint >( this->clone() ) );
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
GuestOverlapConstraint::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Constraint" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "constraint" }
std::vector< std::string >
GuestOverlapConstraint::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"constraint",
	};
}

/// @brief Get the name of this class.
/// @returns "GuestOverlapConstraint".
std::string
GuestOverlapConstraint::class_name() const {
	return "GuestOverlapConstraint";
}

/// @brief Get the namespace for this class.
/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints".
std::string
GuestOverlapConstraint::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the GuestOverlapConstraint class.
masala::base::api::MasalaObjectAPIDefinitionCWP
GuestOverlapConstraint::get_api_definition() {
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
				"The GuestOverlapConstraint class is used to indicate that two guests cannot be seated in the same seat.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( GuestOverlapConstraint, api_def );

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line", "Configure this object from a line in an input file.  "
				"Base class implementation throws.  Must be overridden by derived classes.  This version expects "
				"a line of the form 'GuestOverlapConstraint <constraint_strength>'.",
				"input_line", "The line from which we are configuring this object.  Syntax depends on "
				"derived class.  Must start with an identifier for the constraint type.",
				false, true,
				std::bind( &GuestOverlapConstraint::configure_from_input_line, this, std::placeholders::_1 )
			)
		);

		// Work functions:
		api_def->add_work_function(
			masala::make_shared<
				MasalaObjectAPIWorkFunctionDefinition_FiveInput<
					void,
					seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const &,
					std::vector< std::vector< bool > > const &,
					std::vector< std::map< masala::base::Size, masala::base::Size > > const &,
					masala::numeric::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem &,
					masala::base::Real const
				>
			>(
				"add_constraint_to_cfn_problem", "Modify a pairwise precomputed cost function network optimization problem to add "
				"the constraint to it.  Base class implementation throws.  Must be overridden by derived classes.",
				true, false, true, false,
				"seating_problem", "The object describing the seats, tables, guests, and constraints.",
				"guest_to_allowed_seats", "A vector indexed by guest index, of vectors indexed by seat, of Booleans indicating "
				"whether a seat is allowed for that guest.",
				"guest_to_seat_index_to_guest_choice", "A vector indexed by guest index, of maps of seat index to guest "
				"choice index.  Only the subset of seats accessible to the guest is present in the map.",
				"cfn_problem", "The cost function network optimizaton problem, modified by this operation.",
				"global_strength_multiplier", "A global multiplier for the strength of this constraint.",
				"void", "This function returns nothing.",
				std::bind(
					&GuestOverlapConstraint::add_constraint_to_cfn_problem, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
					std::placeholders::_4, std::placeholders::_5
				)
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
/// @details Base class implementation throws.  Must be overridden by derived classes.  This version expects
/// a line of the form "GuestOverlapConstraint <constraint_strength>".
void
GuestOverlapConstraint::configure_from_input_line(
	std::string const & input_line
) {
	std::istringstream ss( input_line );
	std::string linestart;
	std::lock_guard< std::mutex > lock( mutex() );
	ss >> linestart >> constraint_strength_;
	CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Could not parse line \"" + input_line + "\"." );
	CHECK_OR_THROW_FOR_CLASS( ss.eof(), "configure_from_input_line", "Extra input at end of line \"" + input_line + "\"." );
	CHECK_OR_THROW_FOR_CLASS( linestart == "GuestOverlapConstraint", "configure_from_input_line", "Expected line to begin with class name (\"GuestOverlapConstraint\")." );
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


/// @brief Modify a pairwise precomputed cost function network optimization problem to add
/// the constraint to it.
/// @details Base class implementation throws.  Must be overridden by derived classes.
void
GuestOverlapConstraint::add_constraint_to_cfn_problem(
	seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & seating_problem,
	std::vector< std::vector< bool > > const & guest_to_allowed_seats,
	std::vector< std::map< masala::base::Size, masala::base::Size > > const & guest_to_seat_index_to_guest_choice,
	masala::numeric::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem & cfn_problem,
	masala::base::Real const global_strength_multiplier
) const {
	using masala::base::Size;
	using masala::base::Real;
	using namespace masala::numeric_api::base_classes::optimization::cost_function_network;

	PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem * cfn_problem_cast(
		dynamic_cast< PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem * >( &cfn_problem )
	);
	CHECK_OR_THROW_FOR_CLASS( cfn_problem_cast != nullptr, "add_constraint_to_cfn_problem", "Could not interpret a CFN problem of "
		"type " + cfn_problem.class_name() + " as a PluginPairwisePrecomputedCostFunctionNetworkOptimizationProblem."
	);

	std::lock_guard< std::mutex > lock( mutex() );
	Real const penalty_value( global_strength_multiplier * constraint_strength_ );
	Size const nguests( seating_problem.n_guests() );
	Size const nseats( seating_problem.n_seats() );

	for( Size guest1_index(0); guest1_index < nguests-1; ++guest1_index ) {
		for( Size guest2_index(guest1_index+1); guest2_index < nguests; ++guest2_index ) {
			for( Size iseat(0); iseat < nseats; ++iseat  ) {
				if( guest_to_allowed_seats[guest1_index][iseat] && guest_to_allowed_seats[guest2_index][iseat] ) {
					Size const guest1_choice_index( guest_to_seat_index_to_guest_choice[guest1_index].at(iseat) );
					Size const guest2_choice_index( guest_to_seat_index_to_guest_choice[guest2_index].at(iseat) );
					cfn_problem_cast->add_to_twobody_penalty( std::make_pair( guest1_index, guest2_index ), std::make_pair( guest1_choice_index, guest2_choice_index ), penalty_value );
					write_to_tracer( "Constrained guests " + std::to_string(guest1_index) + " and " + std::to_string(guest2_index) + ", at seats " + std::to_string(iseat) + " and " + std::to_string(iseat) + ".  Penalty: " + std::to_string(penalty_value) + "." );
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
GuestOverlapConstraint::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
GuestOverlapConstraint::protected_assign( SeatingElementBase const & src ) {
	GuestOverlapConstraint const * const src_ptr_cast( dynamic_cast< GuestOverlapConstraint const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a GuestOverlapConstraint object."
	);

	// TODO TODO TODO
	constraint_strength_ = src_ptr_cast->constraint_strength_;

	Parent::protected_assign( src );
}

} // namespace constraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
