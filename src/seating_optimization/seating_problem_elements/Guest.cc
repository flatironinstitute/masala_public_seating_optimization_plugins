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

/// @file src/seating_optimization/seating_problem_elements/Guest.cc
/// @brief Implementations for a Guest.
/// @details A Guest is a person who must be assigned a seat.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit header:
#include <seating_optimization/seating_problem_elements/Guest.hh>

// Numeric headers:
#include <numeric_api/utility/angles/angle_util.hh>

// Base headers:
#include <base/error/ErrorHandling.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/api/constructor/MasalaObjectAPIConstructorMacros.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_OneInput.tmpl.hh>
#include <base/api/setter/MasalaObjectAPISetterDefinition_TwoInput.tmpl.hh>
#include <base/api/getter/MasalaObjectAPIGetterDefinition_ZeroInput.tmpl.hh>
#include <base/utility/string/string_manipulation.hh>

// STL headers:
#include <sstream>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

/// @brief Copy constructor.  Explicit due to mutex.
Guest::Guest( Guest const & src ) :
	Parent( src )
{
	std::lock< std::mutex, std::mutex >( mutex(), src.mutex() );
	std::lock_guard< std::mutex > lockthis( mutex(), std::adopt_lock );
	std::lock_guard< std::mutex > lockthat( src.mutex(), std::adopt_lock );
	Guest::protected_assign( src );
}

/// @brief Make a copy of this object.
seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
Guest::clone() const {
	return masala::make_shared< Guest >( *this );
}

/// @brief Make a fully independent copy of this object.
GuestSP
Guest::deep_clone() const {
	GuestSP new_guest( std::static_pointer_cast< Guest >( this->clone() ) );
	new_guest->make_independent();
	return new_guest;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the category or categories for this plugin class.  Default for all
/// optimization problems; may be overridden by derived classes.
/// @returns { { "SeatingProblem", "SeatingProblemElement", "Guest" } }
/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
/// in more than one hierarchical category (in which case there would be more than one
/// entry in the outer vector), but must be in at least one.  The first one is used as
/// the primary key.
std::vector< std::vector< std::string > >
Guest::get_categories() const {
	return std::vector< std::vector< std::string > > {
		{ "SeatingProblem", "SeatingProblemElement", "Guest" }
	};
}

/// @brief Get the keywords for this plugin class.  Default for all
/// optimization solutions; may be overridden by derived classes.
/// @returns { "seating_problem", "seating_problem_element", "guest" }
std::vector< std::string >
Guest::get_keywords() const {
	return std::vector< std::string > {
		"seating_problem",
		"seating_problem_element",
		"guest",
	};
}

/// @brief Get the name of this class.
/// @returns "Guest".
std::string
Guest::class_name() const {
	return "Guest";
}

/// @brief Get the namespace for this class.
/// @returns "masala::numeric::optimization::cost_function_network".
std::string
Guest::class_namespace() const {
	return "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements";
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

/// @brief Get a description of the API for the Guest class.
masala::base::api::MasalaObjectAPIDefinitionCWP
Guest::get_api_definition() {
	using namespace masala::base::api;
	using namespace masala::base::api::setter;
	using namespace masala::base::api::getter;
	using masala::base::Real;
	using masala::base::Size;

	std::lock_guard< std::mutex > lock( mutex() );

	if( api_definition() == nullptr ) {

		MasalaObjectAPIDefinitionSP api_def(
			masala::make_shared< MasalaObjectAPIDefinition >(
				*this,
				"The Guest class stores all information associated with a person in a seating problem who must be assigned a seat.",
				false, false
			)
		);
		ADD_PUBLIC_CONSTRUCTOR_DEFINITIONS( Guest, api_def );

		// Work functions:

		// Getters:
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< std::string const & > > (
				"name", "Get the full name of this guest.",
				"name", "The full name of this guest.",
				false, false,
				std::bind( &Guest::name, this )
			)
		);
		api_def->add_getter(
			masala::make_shared< MasalaObjectAPIGetterDefinition_ZeroInput< std::string const & > > (
				"unique_identifier", "Get a short string lacking whitespace that serves as a unique identifier for the guest.",
				"unique_identifier", "A short string lacking whitespace that serves as a unique identifier for the guest.",
				false, false,
				std::bind( &Guest::unique_identifier, this )
			)
		);

		// Setters:
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > >(
				"configure_from_input_line",
				"Configure this object from a line in a problem definition file.  This override expects a "
				"line of the format 'Guest <unique_id> <names...>'.  There must be at least a surname, "
				"but you can include as many given names as you like.",
				"file_line", "A line from a configuration file.  Should start with the class name.",
				false, true,
				std::bind( &Guest::configure_from_input_line, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > > (
				"set_name", "Set the full name of this guest.",
				"name_in", "The full name of this guest.  Preceding and trailing whitespace will automatically be trimmed.  "
				"Throws if this is an empty string.",
				false, false,
				std::bind( &Guest::set_name, this, std::placeholders::_1 )
			)
		);
		api_def->add_setter(
			masala::make_shared< MasalaObjectAPISetterDefinition_OneInput< std::string const & > > (
				"set_unique_identifier", "Set a short string lacking whitespace that serves as a unique identifier for the guest.",
				"set_unique_identifier_in", "A short string lacking whitespace that serves as a unique identifier for the guest..  Preceding and trailing whitespace will automatically be trimmed.  "
				"Throws if this is an empty string, or if this contains whitespace.",
				false, false,
				std::bind( &Guest::set_unique_identifier, this, std::placeholders::_1 )
			)
		);

		api_definition() = api_def; //Make const.
	}

	return api_definition();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Get the guest's full name.
std::string const &
Guest::name() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return name_;
}

/// @brief Get a short string lacking whitespace that serves as a unique identifier for the guest.
std::string const &
Guest::unique_identifier() const {
	std::lock_guard< std::mutex > lock( mutex() );
	return unique_identifier_;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

/// @brief Configure this object from a line in a problem definition file.
/// @details This override expects a line of the format "Guest <unique_id> <names...>".  There must be at
/// least a surname, but you can include as many given names as you like.
void
Guest::configure_from_input_line(
	std::string const & file_line
) {
	std::string const linetrimmed( masala::base::utility::string::trim(file_line) );
	std::istringstream ss(linetrimmed);
	std::string tempname;
	std::lock_guard< std::mutex > lock( mutex() );
	ss >> tempname >> unique_identifier_ >> name_;
	CHECK_OR_THROW_FOR_CLASS( !(ss.fail() || ss.bad()), "configure_from_input_line", "Could not parse line \""
		+ linetrimmed + "\".  Expected: Guest <unique_identifier> <names...>"
	);
	CHECK_OR_THROW_FOR_CLASS( tempname == "Guest", "configure_from_input_line", "Expected line \"" + linetrimmed
		+"\" to start with \"Guest\"."
	);
	while( !ss.eof() ) {
		std::string temp;
		ss >> temp;
		std::string const temptrimmed( masala::base::utility::string::trim(temp) );
		if( !temptrimmed.empty() ) {
			if( !name_.empty() ) { name_ += " "; }
			name_ += temptrimmed;
		}
	}
}

/// @brief Set the guest's full name.
void
Guest::set_name(
	std::string const & name_in
) {
	std::string const name_in_stripped( masala::base::utility::string::trim( name_in ) );
	CHECK_OR_THROW_FOR_CLASS( !name_in_stripped.empty(), "set_name", "Name cannot be empty!" );
	std::lock_guard< std::mutex > lock( mutex() );
	name_ = name_in_stripped;
}


/// @brief Set a short string lacking whitespace that serves as a unique identifier for the guest.
void
Guest::set_unique_identifier(
	std::string const &identifier_in
) {
	std::string const identifier_in_stripped( masala::base::utility::string::trim( identifier_in ) );
	CHECK_OR_THROW_FOR_CLASS( !identifier_in_stripped.empty(), "set_unique_identifier", "The unique identifier cannot be empty!" );
	for( Size i(0); i<identifier_in_stripped.size(); ++i ) {
		CHECK_OR_THROW_FOR_CLASS( identifier_in[i] != ' ' && identifier_in[i] != '\t' && identifier_in[i] != '\n' && identifier_in[i] != '\r',
			"set_unique_identifier", "The unique identifier cannot contain whitespace.  \"" + identifier_in_stripped + "\" is invalid."
		);
	}
	std::lock_guard< std::mutex > lock( mutex() );
	unique_identifier_ = identifier_in_stripped;
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
Guest::protected_make_independent() {
	// TODO DEEP CLONING
	Parent::protected_make_independent();
}

/// @brief Assign src to this object.  Derived classes must override this, and the override must call
/// the parent class implementation.
void
Guest::protected_assign( SeatingElementBase const & src ) {
	Guest const * const src_ptr_cast( dynamic_cast< Guest const * >( &src ) );
	CHECK_OR_THROW_FOR_CLASS( src_ptr_cast != nullptr, "protected_assign", "Cannot assign an object of type " + src.class_name()
		+ " to a Guest object."
	);

	// TODO TODO TODO
	name_ = src_ptr_cast->name_;
	unique_identifier_ = src_ptr_cast->unique_identifier_;

	Parent::protected_assign( src );
}

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins
