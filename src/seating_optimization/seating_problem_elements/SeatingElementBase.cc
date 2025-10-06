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

/// @file src/seating_optimization/seating_problem_elements/SeatingElementBase.cc
/// @brief Implementations for SeatingElementBase.
/// @details The SeatingElementBase base class is a common base class for all seating elements.  It is
/// not meant to be instantiated outside of the API definition system.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>

// Numeric headers:

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
SeatingElementBase::SeatingElementBase( SeatingElementBase const & src ) :
    Parent( src )
{
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    SeatingElementBase::protected_assign( src );
}

/// @brief Assignment operator.  Explicit due to mutex.
SeatingElementBase &
SeatingElementBase::operator=(
    SeatingElementBase const & src
) {
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    protected_assign( src );
    return *this;
}


/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
SeatingElementBase::clone() const {
    return masala::make_shared< SeatingElementBase >( *this );
}

/// @brief Make a fully independent copy of this object.
SeatingElementBaseSP
SeatingElementBase::deep_clone() const {
    SeatingElementBaseSP new_seat( std::static_pointer_cast< SeatingElementBase >( this->clone() ) );
    new_seat->make_independent();
    return new_seat;
}

/// @brief Make this object fully independent by deep-cloning all of its sub-objects.
void
SeatingElementBase::make_independent() {
    std::lock_guard< std::mutex > lockthis( mutex_ );
    protected_make_independent();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "OptimizationSolution", "CostFunctionNetworkOptimizationSolution", "SeatingElementBase" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
SeatingElementBase::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "SeatingElementBase" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "optimization_solution", "qubo_solution", "numeric" }
std::vector< std::string >
SeatingElementBase::get_keywords() const {
	return std::vector< std::string > {
        "seating_problem",
		"seating_problem_element",
        "seat",
	};
}

/// @brief Get the name of this class.
/// @returns "SeatingElementBase".
std::string
SeatingElementBase::class_name() const {
    return "SeatingElementBase";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
SeatingElementBase::class_namespace() const {
    return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the SeatingElementBase class.
masala::base::api::MasalaObjectAPIDefinitionCWP
SeatingElementBase::get_api_definition() {
    using namespace masala::base::api;
    using namespace masala::base::api::setter;
    using masala::base::Real;
    using masala::base::Size;

    std::lock_guard< std::mutex > lock( mutex_ );

    if( api_definition() == nullptr ) {

        MasalaObjectAPIDefinitionSP api_def(
            masala::make_shared< MasalaObjectAPIDefinition >(
                *this,
                "The SeatingElementBase class is a common base class for all seating elements.  It is not intended to be "
                "instantiated outside of the build system.",
                false, true
            )
        );
        ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( SeatingElementBase, api_def );

        // Work functions:

        // Getters:

        // Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line",
				"Configure this object from a line in a problem definition file.  Must be "
				"implemented by derived classes.  Base class implementation throws.",
				"file_line", "A line from a configuration file.  Should start with the class name.",
				true, false,
				std::bind( &SeatingElementBase::configure_from_input_line, this, std::placeholders::_1 )
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

/// @brief Configure this object from a line in a problem definition file.
/// @details Must be implemented by derived classes.  Base class implementation throws.
void
SeatingElementBase::configure_from_input_line(
	std::string const & file_line
) {
	MASALA_THROW( class_namespace() + "::" + class_name(), "configure_from_input_line",
		"Error parsing line \"" + file_line + "\".  This class has not overridden "
		"this function.  Please consult a developer, as this is a program error that "
		"ought not to happen."
	);
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingElementBase::protected_make_independent() {
    /* GNDN */
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingElementBase::protected_assign(
    SeatingElementBase const & /*src*/
) {
    /* GNDN */
}

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins