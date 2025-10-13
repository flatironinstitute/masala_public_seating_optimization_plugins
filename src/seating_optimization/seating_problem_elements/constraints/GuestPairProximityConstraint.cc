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

/// @file src/seating_optimization/seating_problem_elements/constraints/GuestPairProximityConstraint.cc
/// @brief Implementations for a GuestPairProximityConstraint.
/// @details The GuestPairProximityConstraint class is used to indicate that two guests should be close together or should not be close together.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/constraints/GuestPairProximityConstraint.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem/SeatingProblem.hh>
#include <seating_optimization/seating_problem_elements/Seat.hh>

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

/// @brief Given the falloff mode as an enum, get the string.
std::string
falloff_mode_string_from_enum(
	ProximityFalloffMode const mode_enum
) {
	switch(mode_enum) {
		case ProximityFalloffMode::GAUSSIAN :
			return "GAUSSIAN";
		default :
			break;
	}

	MASALA_THROW( "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints", "falloff_mode_string_from_enum",
		"Invalid enum passed to this function."
	);
	return "";
}

/// @brief Given the falloff mode as a string, get the enum.
/// @details Returns ProximityFalloffMode::INVALID_MODE if the string can't be interpreted as a proximity mode.
ProximityFalloffMode
falloff_mode_enum_from_string(
	std::string const & modestring
) {
	using masala::base::Size;
	for( Size i(1); i<static_cast<Size>(ProximityFalloffMode::N_FALLOFF_MODES); ++i ) {
		if( modestring == falloff_mode_string_from_enum( static_cast<ProximityFalloffMode>(i) ) ) {
			return static_cast<ProximityFalloffMode>(i);
		}
	}
	return ProximityFalloffMode::INVALID_MODE;
}

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
GuestPairProximityConstraint::GuestPairProximityConstraint( GuestPairProximityConstraint const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	GuestPairProximityConstraint::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
GuestPairProximityConstraint::clone() const {
	return masala::make_shared< GuestPairProximityConstraint >( *this );
}

/// @brief Make a fully independent copy of this object.
GuestPairProximityConstraintSP
GuestPairProximityConstraint::deep_clone() const {
	GuestPairProximityConstraintSP new_guest( std::static_pointer_cast< GuestPairProximityConstraint >( this->clone() ) );
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
GuestPairProximityConstraint::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Constraint" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "constraint" }
std::vector< std::string >
GuestPairProximityConstraint::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"constraint",
	};
}

/// @brief Get the name of this class.
/// @returns "GuestPairProximityConstraint".
std::string
GuestPairProximityConstraint::class_name() const {
	return "GuestPairProximityConstraint";
}

/// @brief Get the namespace for this class.
/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints".
std::string
GuestPairProximityConstraint::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the GuestPairProximityConstraint class.
masala::base::api::MasalaObjectAPIDefinitionCWP
GuestPairProximityConstraint::get_api_definition() {
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
				"The GuestPairProximityConstraint class is used to indicate that two guests should be close together or should not be close together.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( GuestPairProximityConstraint, api_def );

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line", "Configure this object from a line in an input file.  "
				"Base class implementation throws.  Must be overridden by derived classes.  This version expects "
				"a line of the form 'GuestPairProximityConstraint <guest1_uid> <guest2_uid> <constraint_strength_at_one_unit> <falloff_type> <falloff_options...>'.",
				"input_line", "The line from which we are configuring this object.  Syntax depends on "
				"derived class.  Must start with an identifier for the constraint type.",
				false, true,
				std::bind( &GuestPairProximityConstraint::configure_from_input_line, this, std::placeholders::_1 )
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
					&GuestPairProximityConstraint::add_constraint_to_cfn_problem, this,
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
/// a line of the form "GuestPairProximityConstraint <guest1_uid> <guest2_uid> <constraint_strength_at_one_unit> <falloff_type> <falloff_options...>".
void
GuestPairProximityConstraint::configure_from_input_line(
	std::string const & input_line
) {
	std::istringstream ss( input_line );
	std::string linestart;
	std::lock_guard< std::mutex > lock( mutex() );

	ss >> linestart >> first_guest_uid_ >> second_guest_uid_ >> constraint_strength_at_one_unit_;
	CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Could not parse line \"" + input_line + "\"." );
	CHECK_OR_THROW_FOR_CLASS( linestart == "GuestPairProximityConstraint", "configure_from_input_line", "Expected line to begin with class name (\"GuestPairProximityConstraint\")." );

	std::string falloff_mode_str;
	ss >> falloff_mode_str;
	CHECK_OR_THROW_FOR_CLASS( !(ss.bad() || ss.fail()), "configure_from_input_line", "Could not parse line \"" + input_line + "\"." );

	falloff_mode_ = falloff_mode_enum_from_string( falloff_mode_str );

	switch( falloff_mode_ ) {
		case ProximityFalloffMode::INVALID_MODE :
			MASALA_THROW( class_namespace() + "::" + class_name(), "configure_from_input_line", "Could not parse \"" + falloff_mode_str + "\" as a valid falloff mode." );
			break;
		case ProximityFalloffMode::GAUSSIAN :
			protected_parse_gaussian_falloff_mode( ss, input_line );
			break;
	}

	CHECK_OR_THROW_FOR_CLASS( ss.eof(), "configure_from_input_line", "Extra input at end of line \"" + input_line + "\"." );
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


/// @brief Modify a pairwise precomputed cost function network optimization problem to add
/// the constraint to it.
/// @details Base class implementation throws.  Must be overridden by derived classes.
void
GuestPairProximityConstraint::add_constraint_to_cfn_problem(
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
	Size guest1_index( seating_problem.guest_index_from_uid( first_guest_uid_ ) );
	Size guest2_index( seating_problem.guest_index_from_uid( second_guest_uid_ ) );
	if( guest2_index < guest1_index ) {
		std::swap( guest1_index, guest2_index );
	}
	std::pair< Size, Size > guestpair( guest1_index, guest2_index );
	Real const penalty_value_at_one_unit( global_strength_multiplier * constraint_strength_at_one_unit_ );
	Size const nseats( seating_problem.n_seats() );

	for( Size iseat(0); iseat < nseats; ++iseat ) {
		if( !guest_to_allowed_seats[guest1_index][iseat] ) {
			continue;
		}
		Size const guest1_choice_index( guest_to_seat_index_to_guest_choice[guest1_index].at(iseat) );
		SeatCSP iseat_object( seating_problem.seat(iseat) );
		std::pair< Real, Real > const seat1coord( std::make_pair( iseat_object->x(), iseat_object->y() ) );
		for( Size jseat(0); jseat < nseats; ++jseat ) {
			if( !guest_to_allowed_seats[guest2_index][jseat] ) {
				continue;
			}
			Size const guest2_choice_index( guest_to_seat_index_to_guest_choice[guest2_index].at(jseat) );
			SeatCSP jseat_object( seating_problem.seat(jseat) );
			std::pair< Real, Real > const seat2coord( std::make_pair( jseat_object->x(), jseat_object->y() ) );
			Real const curpenalty( protected_compute_penalty( seat1coord, seat2coord, penalty_value_at_one_unit ) );
			cfn_problem_cast->add_to_twobody_penalty( guestpair, std::make_pair( guest1_choice_index, guest2_choice_index ), curpenalty );
			write_to_tracer( "Proximity-constrained guests " + std::to_string(guest1_index) + " and " + std::to_string(guest2_index)
				+ ", at seats " + std::to_string(iseat) + " and " + std::to_string(jseat) + ".  Penalty: " + std::to_string(curpenalty) + "."
			);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
GuestPairProximityConstraint::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
GuestPairProximityConstraint::protected_assign( SeatingElementBase const & src ) {
	GuestPairProximityConstraint const * const src_ptr_cast( dynamic_cast< GuestPairProximityConstraint const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a GuestPairProximityConstraint object."
	);

	// TODO TODO TODO
	first_guest_uid_ = src_ptr_cast->first_guest_uid_;
	second_guest_uid_ = src_ptr_cast->second_guest_uid_;
	constraint_strength_at_one_unit_ = src_ptr_cast->constraint_strength_at_one_unit_;
	falloff_mode_ = src_ptr_cast->falloff_mode_;
	gaussian_sd_ = src_ptr_cast->gaussian_sd_;

	Parent::protected_assign( src );
}

/// @brief Parse a Gaussian setup, of the form "GAUSSIAN <gaussian_sd>."
void
GuestPairProximityConstraint::protected_parse_gaussian_falloff_mode(
	std::istringstream & ss,
	std::string const & input_line
) {
	ss >> gaussian_sd_;
	CHECK_OR_THROW_FOR_CLASS( gaussian_sd_ > 0, "protected_parse_gaussian_falloff_mode",
			"The Gaussian standard deviation must be greater than zero.  Error parsing line \"" + input_line + "\"."
	);
}

/// @brief Compute the penalty.  Performs no mutex-locking, so should be called from a mutex-locked context.
masala::base::Real
GuestPairProximityConstraint::protected_compute_penalty(
	std::pair< masala::base::Real, masala::base::Real > const & seat1coord,
	std::pair< masala::base::Real, masala::base::Real > const & seat2coord,
	masala::base::Real penalty_value_at_one_unit
) const {
	using masala::base::Real;
	switch( falloff_mode_ ) {
		case ProximityFalloffMode::INVALID_MODE :
			MASALA_THROW( class_namespace() + "::" + class_name(), "protected_compute_penalty", "Invalid falloff mode selected." );
		case ProximityFalloffMode::GAUSSIAN :
		{
			// Not as efficient as it could be, but it doesn't matter for the paltry number of times that this function will be called.
			Real const distsq( std::pow(seat1coord.first - seat2coord.first, 2) + std::pow(seat1coord.second - seat2coord.second, 2) );
			Real const gaussian_sd_sq( gaussian_sd_ * gaussian_sd_ );
			return penalty_value_at_one_unit / std::exp( -1/gaussian_sd_sq ) * std::exp( -distsq / gaussian_sd_sq );
		}
	}

	MASALA_THROW( class_namespace() + "::" + class_name(), "protected_compute_penalty", "Invalid falloff mode selected." );

	return 0.0;
}

} // namespace constraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
