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
#include <seating_optimization/seating_problem/SeatingProblem.hh>
#include <seating_optimization/seating_problem_elements/Guest.hh>
#include <seating_optimization/seating_problem_elements/Seat.hh>

// Base headers:
#include <base/utility/string/string_manipulation.hh>
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_OneInput.tmpl.hh>
#include <base/api/work_function/MasalaObjectAPIWorkFunctionDefinition_ZeroInput.tmpl.hh>
#include <base/managers/plugin_module/MasalaPluginAPI.hh>

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
		api_def->add_work_function(
			masala::make_shared< MasalaObjectAPIWorkFunctionDefinition_OneInput< void, bool const > >(
				"print_solution", "Print the solution to the tracer.  Must be finalized first.  If include_problem is true, "
				"all information needed to visualize the solution (including coordinates of chairs and tables) is printed.",
				true, false, false, false,
				"include_problem", "If true, a description of the problem is printed.",
				"void", "This function returns nothing.",
				std::bind( &SeatingSolution::print_solution, this, std::placeholders::_1 )
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
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< SeatingProblemCSP > >(
				"set_problem", "Set the problem for which this is a solutuion.  Problem is used directly, not deep-cloned.",
				"problem_in", "The problem for which this is a solution.",
				false, false,
				std::bind( &SeatingSolution::set_problem, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_TwoInput< Size, Size > >(
				"add_guest_seat_assignment", "Mark that a particular guest is assigned a particular seat.  Solution must not "
                "yet be finalized.",
				"guest_index", "The zero-based index of the guest to be assigned a seat.",
                "seat_index", "The zero-based index of the seat that this guest has been assigned.",
				false, false,
				std::bind( &SeatingSolution::add_guest_seat_assignment, this, std::placeholders::_1, std::placeholders::_2 )
			)
		);

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

/// @brief Set the problem for which this is a solutuion.  Problem is used directly, not deep-cloned.
void
SeatingSolution::set_problem(
	SeatingProblemCSP problem
) {
	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "set_problem", "This object must not be finalized before this function is called." );
	seating_problem_ = problem;
}

/// @brief Mark that a particular guest is assigned a particular seat.  Solution must not
/// yet be finalized.
void
SeatingSolution::add_guest_seat_assignment(
    masala::base::Size const guest_index,
    masala::base::Size const seat_index
) {
    std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "add_guest_seat_assignment", "This object must not be finalized before this function is called." );
    CHECK_OR_THROW_FOR_CLASS( guest_index_to_seat_index_.count(guest_index) == 0, "add_guest_seat_assignment", "Guest with index " + std::to_string(guest_index) + " has already been assigned a seat." );
    guest_index_to_seat_index_[guest_index] = seat_index;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Mark this object as having been finalized.
void
SeatingSolution::finalize() {
    using masala::base::Size;
    using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( !finalized_, "finalize", "This object has already been finalized." );
	CHECK_OR_THROW_FOR_CLASS( seating_problem_ != nullptr, "finalize", "A seating problem definition must be provided before this object is finalized." );
    Size const nguest( seating_problem_->n_guests() );
    CHECK_OR_THROW_FOR_CLASS( nguest == guest_index_to_seat_index_.size(), "finalize", "Expected seat assignments for all guests to be loaded into this object prior to finalization." );

    for( auto const & entry : guest_index_to_seat_index_ ) {
        CHECK_OR_THROW_FOR_CLASS( entry.first < nguest, "finalize", "Expected guest indices to be less than " + std::to_string(nguest) +
            ", but got " + std::to_string(entry.first) + "."
        );
        GuestCSP curguest( seating_problem_->guest(entry.first));
        SeatCSP curseat( seating_problem_->seat(entry.second) );
        CHECK_OR_THROW_FOR_CLASS( guest_to_seat_.count( curguest ) == 0, "finalize", "Guest \"" + curguest->name() + "\" (UID \"" + curguest->unique_identifier() + "\") was added more than once." );
        guest_to_seat_[curguest] = curseat;
    }

	finalized_ = true;
}

/// @brief Print the solution to the tracer.  Must be finalized first.  If include_problem is true,
/// all information needed to visualize the solution (including coordinates of chairs and tables) is
/// printed.
void
SeatingSolution::print_solution(
	bool const include_problem
) const {
	using masala::base::Size;
	using namespace seating_optimization_masala_plugins::seating_optimization::seating_problem_elements;

	std::lock_guard< std::mutex > lock( mutex_ );
	CHECK_OR_THROW_FOR_CLASS( finalized_, "print_solution", "This object must be finalized before this function is called." );
	CHECK_OR_THROW_FOR_CLASS( seating_problem_ != nullptr, "print_solution", "A seating problem definition must be provided before this function is called." );
	if( include_problem ) {
		write_to_tracer("PROBLEM:");
		seating_problem_->print_problem();
		write_to_tracer("");
		write_to_tracer( "SOLUTION:" );
	}
	write_to_tracer( "Guest_index\tGuest_UID\tGuest_name\tSeat_global_index\tTable_index\tSeat_index_at_table" );

	masala::base::Size counter(0);
	for( auto const & entry : guest_to_seat_ ) {
		Guest const & guest( *entry.first );
		Size const seat_index( guest_index_to_seat_index_.at(counter) );

		bool const seat_is_at_a_table( seating_problem_->seat_is_at_a_table( seat_index ) );
		std::pair< Size, Size > table_and_local_seat_index( seating_problem_->table_and_local_seat_index_from_global_seat_index( seat_index ) );

		write_to_tracer(
			std::to_string(counter)
			+ "\t" + guest.unique_identifier()
			+ "\t\"" + guest.name() + "\""
			+ "\t" + std::to_string( seat_index )
			+ "\t" + ( seat_is_at_a_table ? std::to_string( table_and_local_seat_index.first ) : "N/A" )
			+ "\t" + ( seat_is_at_a_table ? std::to_string( table_and_local_seat_index.second ) : "N/A" )
		);
		++counter;
	}
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
	if( seating_problem_ != nullptr ) {
		SeatingProblemSP problem_copy( seating_problem_->clone() );
		problem_copy->make_independent();
		seating_problem_ = problem_copy;
	}
    auto const guest_to_seat_copy(guest_to_seat_);
    guest_to_seat_.clear();
    for( auto const & entry : guest_to_seat_copy ) {
        CHECK_OR_THROW_FOR_CLASS( entry.first != nullptr && entry.second != nullptr, "protected_make_independent", "Invalid entry in guest_to_seat_ map." );
        GuestSP guest_clone( std::dynamic_pointer_cast< Guest >( entry.first->clone() ) );
        CHECK_OR_THROW_FOR_CLASS( guest_clone != nullptr, "protected_make_independent", "Failed to clone guest." );
        guest_clone->make_independent();
        SeatSP seat_clone( std::dynamic_pointer_cast< Seat >( entry.second->clone() ) );
        CHECK_OR_THROW_FOR_CLASS( seat_clone != nullptr, "protected_make_independent", "Failed to clone seat." );
        seat_clone->make_independent();
        guest_to_seat_[guest_clone] = seat_clone;
    }
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
SeatingSolution::protected_assign(
    SeatingSolution const & src
) {
	finalized_ = src.finalized_;
	seating_problem_ = src.seating_problem_;
    guest_index_to_seat_index_ = src.guest_index_to_seat_index_;
    guest_to_seat_ = src.guest_to_seat_;
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins