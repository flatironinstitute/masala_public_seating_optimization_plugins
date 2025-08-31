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

/// @file src/seating_optimization/seating_problem/SeatingProblem.cc
/// @brief Implementations for SeatingProblem.
/// @details The SeatingProblem describes a seating problem, including the tables, the seats, the guests, and the constraints.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem/SeatingProblem.hh>

// Numeric headers:

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>

// STL headers:

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
SeatingProblem::SeatingProblem( SeatingProblem const & src ) :
    Parent( src )
{
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    SeatingProblem::protected_assign( src );
}

/// @brief Assignment operator.  Explicit due to mutex.
SeatingProblem &
SeatingProblem::operator=(
    SeatingProblem const & src
) {
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    protected_assign( src );
    return *this;
}


/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblemSP
SeatingProblem::clone() const {
    return masala::make_shared< SeatingProblem >( *this );
}

/// @brief Make a fully independent copy of this object.
SeatingProblemSP
SeatingProblem::deep_clone() const {
    SeatingProblemSP new_seat( std::static_pointer_cast< SeatingProblem >( this->clone() ) );
    new_seat->make_independent();
    return new_seat;
}

/// @brief Make this object fully independent by deep-cloning all of its sub-objects.
void
SeatingProblem::make_independent() {
    std::lock_guard< std::mutex > lockthis( mutex_ );
    protected_make_independent();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
SeatingProblem::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem" }
std::vector< std::string >
SeatingProblem::get_keywords() const {
	return std::vector< std::string > {
        "seating_problem"
	};
}

/// @brief Get the name of this class.
/// @returns "SeatingProblem".
std::string
SeatingProblem::class_name() const {
    return "SeatingProblem";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
SeatingProblem::class_namespace() const {
    return "seating_optimization_masala_plugins::seating_optimization::seating_problem";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the SeatingProblem class.
masala::base::api::MasalaObjectAPIDefinitionCWP
SeatingProblem::get_api_definition() {
    using namespace masala::base::api;
    using masala::base::Real;
    using masala::base::Size;

    std::lock_guard< std::mutex > lock( mutex_ );

    if( api_definition() == nullptr ) {

        MasalaObjectAPIDefinitionSP api_def(
            masala::make_shared< MasalaObjectAPIDefinition >(
                *this,
                "The SeatingProblem describes a seating problem, including the tables, the seats, the guests, and the constraints.",
                false, true
            )
        );
        ADD_PROTECTED_CONSTRUCTOR_DEFINITIONS( SeatingProblem, api_def );

        // Work functions:

        // Getters:

        // Setters:

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

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingProblem::protected_make_independent() {
    /* GNDN */
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingProblem::protected_assign(
    SeatingProblem const & /*src*/
) {
    /* GNDN */
}

} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins