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

/// @file src/seating_optimization/seating_problem/SeatingProblem.hh
/// @brief Headers for a SeatingProblem.
/// @details The SeatingProblem describes a seating problem, including the tables, the seats, the guests, and the constraints.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_SeatingProblem_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_SeatingProblem_hh

// Forward declarations:
#include <seating_optimization/seating_problem/SeatingProblem.fwd.hh>

// Parent header:
#include <base/managers/plugin_module/MasalaPlugin.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Guest.fwd.hh>
#include <seating_optimization/seating_problem_elements/Table.fwd.hh>
#include <seating_optimization/seating_problem_elements/Seat.fwd.hh>

// Base headers:
#include <base/types.hh>

// STL headers:
#include <mutex>
#include <map>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem {

/// @brief A SeatingProblem.
/// @details The SeatingProblem describes a seating problem, including the tables, the seats, the guests, and the constraints.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class SeatingProblem : public masala::base::managers::plugin_module::MasalaPlugin {

	typedef masala::base::managers::plugin_module::MasalaPlugin Parent;
	typedef masala::base::managers::plugin_module::MasalaPluginSP ParentSP;
	typedef masala::base::managers::plugin_module::MasalaPluginCSP ParentCSP;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	SeatingProblem() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	SeatingProblem( SeatingProblem const & src );

	/// @brief Assignment operator.  Explicit due to mutex.
	SeatingProblem & operator=( SeatingProblem const & src );

	/// @brief Destructor.
	~SeatingProblem() override = default;

	/// @brief Make a copy of this object.
	virtual
	seating_optimization_masala_plugins::seating_optimization::seating_problem::SeatingProblemSP
	clone() const;

	/// @brief Make a fully independent copy of this object.
	SeatingProblemSP
	deep_clone() const;

	/// @brief Make this object fully independent by deep-cloning all of its sub-objects.
	void make_independent();

public:

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
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "seating_problem" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "SeatingProblem".
	std::string
	class_name() const override;

	/// @brief Get the namespace for this class.
	/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem".
	std::string
	class_namespace() const override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

    /// @brief Get a description of the API for the CostFunctionNetworkOptimizationSolution class.
    masala::base::api::MasalaObjectAPIDefinitionCWP
    get_api_definition() override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Given a unique guest identifier, get the guest index.  Throws if guest not found.  Indices are zero-based.
	masala::base::Size guest_index_from_uid( std::string const & guest_unique_identifier ) const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.
	void
	add_guest(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & guest_in
	);

	/// @brief Add a table.  Stored directly; not cloned.
	void
	add_table(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP const & table_in
	);

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

protected:

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Deried classes can lock the mutex.
	std::mutex & mutex() const { return mutex_; }

	/// @brief Derived classes can access the API definition.
	masala::base::api::MasalaObjectAPIDefinitionCSP &
	api_definition() {
		return api_definition_;
	}

	/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
	/// the parent class implementation.
	virtual void protected_make_independent();

	/// @brief Assign src to this object.  Derived classes must override this, and the override must call
	/// the parent class implementation.
	virtual void protected_assign( SeatingProblem const & src );

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Update the list of seat indices (from scratch).
	/// @details The seat_indices_ map is cleared and repopulated by this operation.  No mutex locking
	void regenerate_seat_indices();

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
////////////////////////////////////////////////////////////////////////////////

	/// @brief A mutex for this object and its children.
	mutable std::mutex mutex_;

	/// @brief The API definition for this object.s
	masala::base::api::MasalaObjectAPIDefinitionCSP api_definition_;

	/// @brief The guests that we are storing, stored by unique identifier.
	/// @details The UID string points to a pair of guest index, guest object.
	std::map< std::string, std::pair< masala::base::Size, seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP > > guests_;

	/// @brief The tables, stored by index.
	std::vector< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP > tables_;

	/// @brief The zero-based absolute indices of the seats.
	std::map< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP, masala::base::Size > seat_indices_;

}; // class SeatingProblem

} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_SeatingProblem_hh