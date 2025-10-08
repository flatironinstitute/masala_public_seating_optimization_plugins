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

/// @file src/seating_optimization/seating_problem/SeatingSolution.cc
/// @brief Implementations for SeatingSolution.
/// @details The SeatingSolution describes the solution to a seating problem.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem/SeatingSolution.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Guest.hh>
#include <seating_optimization/seating_problem_elements/Table.hh>
#include <seating_optimization/seating_problem_elements/Seat.hh>
#include <seating_optimization/seating_problem_elements/SeatingElementBase.hh>
#include <seating_optimization/seating_problem_elements/constraints/Constraint.hh>

// Base headers:
#include <base/utility/string/string_manipulation.hh>
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_OneInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ZeroInput.tmpl.hh>
#include <base/managers/plugin_module/MasalaPluginAPI.hh>
#include <base/managers/plugin_module/MasalaPluginModuleManager.hh>

// Numeric headers:
#include <numeric/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem.hh>

// Numeric API headers:
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem_API.hh>

// STL headers:
#include <sstream>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
SeatingSolution::SeatingSolution( SeatingSolution const & src ) :
    Parent( src )
{
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    SeatingSolution::protected_assign( src );
}

/// @brief Assignment operator.  Explicit due to mutex.
SeatingSolution &
SeatingSolution::operator=(
    SeatingSolution const & src
) {
    std::lock< std::mutex, std::mutex >( mutex_, src.mutex_ );
    std::lock_guard< std::mutex > lockthis( mutex_, std::adopt_lock );
    std::lock_guard< std::mutex > lockthat( src.mutex_, std::adopt_lock );
    protected_assign( src );
    return *this;
}


/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingSolutionSP
SeatingSolution::clone() const {
    return masala::make_shared< SeatingSolution >( *this );
}

/// @brief Make a fully independent copy of this object.
SeatingSolutionSP
SeatingSolution::deep_clone() const {
    SeatingSolutionSP new_seat( std::static_pointer_cast< SeatingSolution >( this->clone() ) );
    new_seat->make_independent();
    return new_seat;
}

/// @brief Make this object fully independent by deep-cloning all of its sub-objects.
void
SeatingSolution::make_independent() {
    std::lock_guard< std::mutex > lockthis( mutex_ );
    protected_make_independent();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingSolution" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
SeatingSolution::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingSolution" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_solution" }
std::vector< std::string >
SeatingSolution::get_keywords() const {
	return std::vector< std::string > {
        "seating_solution"
	};
}

/// @brief Get the name of this class.
/// @returns "SeatingSolution".
std::string
SeatingSolution::class_name() const {
    return "SeatingSolution";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
SeatingSolution::class_namespace() const {
    return "seating_optimization_masala_plugins::seating_optimization::seating_problem";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the SeatingSolution class.
masala::base::api::MasalaObjectAPIDefinitionCWP
SeatingSolution::get_api_definition() {
    using namespace masala::base::api;
    using namespace masala::base::api::setter;
    using namespace masala::base::api::getter;
    using namespace masala::base::api::work_function;
    using masala::base::Real;
    using masala::base::Size;

    std::lock_guard< std::mutex > lock( mutex_ );

    if( api_definition() == nullptr ) {

        MasalaObjectAPIDefinitionSP api_def(
            masala::make_shared< MasalaObjectAPIDefinition >(
                *this,
                "The SeatingSolution describes a seating problem, including the tables, the seats, the guests, and the constraints.",
                false, false
            )
        );
        ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( SeatingSolution, api_def );

        // Work functions:
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_ZeroInput< void > >(
				"finalize", "Mark this object as having been fully configured.",
				false, false, false, false,
				"void", "This function returns nothing.",
				std::bind( &SeatingSolution::finalize, this )
			)
		);

        // Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< bool > >(
				"finalized", "Has this object been finalized?",
				"finalized", "True if this object has been finalized; false otherwise.",
				false, false,
				std::bind( &SeatingSolution::finalized, this )
			)
		);

        // Setters:

        api_definition() = api_def; //Make const.
    }

    return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Has this object been finalized?
bool
SeatingSolution::finalized() const {
	std::lock_guard< std::mutex > lock( mutex_ );
	return finalized_;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Mark this object as having been finalized.
void
SeatingSolution::finalize() {
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "finalize", "This object has already been finalized." );
	finalized_ = true;
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingSolution::protected_make_independent() {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	api_definition_ = nullptr;
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingSolution::protected_assign(
    SeatingSolution const & //src
) {

}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins