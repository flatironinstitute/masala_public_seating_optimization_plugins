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

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Guest.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_OneInput.tmpl.hh>

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
    using namespace masala::base::api::setter;
    using namespace masala::base::api::getter;
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
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_OneInput< Size, std::string const & > >(
				"guest_index_from_uid", "Given a unique guest identifier, get the guest index.  Throws if guest not found.  Indices are zero-based.",
				"guest_unique_identifier", "The unique identifier for the guest in question.",
				"guest_index", "The zero-based index (number) for the guest.",
				false, false,
				std::bind( &SeatingProblem::guest_index_from_uid, this, std::placeholders::_1 )
			)
		);

        // Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & > >(
				"add_guest", "Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.",
				"guest_in", "The guest to add.",
				false, false,
				std::bind( &SeatingProblem::add_guest, this, std::placeholders::_1 )
			)
		);

        api_definition() = api_def; //Make const.
    }

    return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Given a unique guest identifier, get the guest index.  Throws if guest not found.  Indices are zero-based.
masala::base::Size
SeatingProblem::guest_index_from_uid(
	std::string const & guest_unique_identifier
) const {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	std::lock_guard< std::mutex > lock( mutex_ );
	std::map< std::string, std::pair< Size, GuestCSP > >::const_iterator it( guests_.find( guest_unique_identifier ) );
	CHECK_OR_THROW_FOR_CLASS( it != guests_.end(), "guest_index_from_uid", "No unique guest ID \"" + guest_unique_identifier + "\" was found." );
	return it->second.first;
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.
void
SeatingProblem::add_guest(
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & guest_in
) {
	std::string const uid( guest_in->unique_identifier() );
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( guests_.count( uid ) == 0, "add_guest", "A guest with unique identifier \""
		+ uid + "\" has already been added."
	);
	masala::base::Size nguests( guests_.size() );
	guests_[uid] = std::make_pair( nguests, guest_in );
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