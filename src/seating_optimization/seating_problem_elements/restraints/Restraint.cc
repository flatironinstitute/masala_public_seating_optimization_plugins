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

/// @file src/seating_optimization/seating_problem_elements/restraints/Restraint.cc
/// @brief Implementations for a Restraint.
/// @details The Restraint class is the base class for restraints, which are elements of a seating problem.  They stricly limit the
/// seating choices for a particular guest, unlike Constraints, which only impose a penalty or a bonus for a particular seating choice.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/restraints/Restraint.hh>

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
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_TwoInput.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {
namespace restraints {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
Restraint::Restraint( Restraint const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	Restraint::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
Restraint::clone() const {
	return masala::make_shared< Restraint >( *this );
}

/// @brief Make a fully independent copy of this object.
RestraintSP
Restraint::deep_clone() const {
	RestraintSP new_guest( std::static_pointer_cast< Restraint >( this->clone() ) );
	new_guest->make_independent();
	return new_guest;
}

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
Restraint::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Restraint" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "restraint" }
std::vector< std::string >
Restraint::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"restraint",
	};
}

/// @brief Get the name of this class.
/// @returns "Restraint".
std::string
Restraint::class_name() const {
	return "Restraint";
}

/// @brief Get the namespace for this class.
/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints".
std::string
Restraint::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the Restraint class.
masala::base::api::MasalaObjectAPIDefinitionCWP
Restraint::get_api_definition() {
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
				"The Restraint class is the base class for restraints, which are elements of a seating problem.  They stricly limit the "
				"seating choices for a particular guest, unlike Constraints, which only impose a penalty or a bonus for a particular seating "
				"choice.", false, true
			)
		);
		ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( Restraint, api_def );

		// Getters:

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line", "Configure this object from a line in an input file.  "
				"Base class implementation throws.  Must be overridden by derived classes.",
				"input_line", "The line from which we are configuring this object.  Syntax depends on "
				"derived class.  Must start with an identifier for the restraint type.",
				false, true,
				std::bind( &Restraint::configure_from_input_line, this, std::placeholders::_1 )
			)
		);

		// Work functions:
		api_def->add_work_function(
			masala::make_shared<
				MasalaObjectAPIWorkFunctionDefinition_TwoInput<
					void,
					seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const &,
					std::vector< std::vector< bool > > &
				>
			>(
				"restrain_seating_choices", "Limit the allowed seating choices for the guests.  Base class implementation throws.  "
				"Must be overridden by derived classes.  The allowed_seating_choices_by_guest vector is a vector with one entry per "
				"guest, where each entry is a Boolean vector with one entry per seat.  Restraints can set some subset of these Booleans "
				"to false.",
				true, false, true, false,
				"seating_problem", "The object describing the seats, tables, guests, and restraints.",
				"allowed_seating_choices_by_guest", "A vector of vectors of Booleans, where the outer vector is indexed by guest and the "
				"inner vector is indexed by seat global index.  True means that this guest can sit here, and false means they cannot.",
				"void", "This function returns nothing.",
				std::bind( &Restraint::restrain_seating_choices, this, std::placeholders::_1, std::placeholders::_2 )
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
void
Restraint::configure_from_input_line(
	std::string const & //input_line
) {
	MASALA_THROW( class_namespace() + "::" + class_name(), "configure_from_input_line", "This restraint class must override this function." );
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Limit the allowed seating choices for the guests.
/// @details Base class implementation throws.  Must be overridden by derived classes.  The allowed_seating_choices_by_guest
/// vector is a vector with one entry per guest, where each entry is a Boolean vector with one entry per seat.  Restraints can
/// set some subset of these Booleans to false.
/*virtual*/
void
Restraint::restrain_seating_choices(
	seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblem const & seating_problem,
	std::vector< std::vector< bool > > & allowed_seating_choices_by_guest
) const {
	MASALA_THROW( class_namespace() + "::" + class_name(), "restrain_seating_choices", "This class must override this function." );
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Restraint::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Restraint::protected_assign( SeatingElementBase const & src ) {
	Restraint const * const src_ptr_cast( dynamic_cast< Restraint const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a Restraint object."
	);

	// TODO TODO TODO

	Parent::protected_assign( src );
}

} // namespace restraints
} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
