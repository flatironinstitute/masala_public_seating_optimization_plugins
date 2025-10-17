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

// Numeric API headers:
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationProblem_API.fwd.hh>
#include <numeric_api/auto_generated_api/optimization/cost_function_network/CostFunctionNetworkOptimizationSolution_API.fwd.hh>

// Seating optimization headers:
#include <seating_optimization/seating_problem_elements/Guest.fwd.hh>
#include <seating_optimization/seating_problem_elements/Table.fwd.hh>
#include <seating_optimization/seating_problem_elements/Seat.fwd.hh>
#include <seating_optimization/seating_problem_elements/Room.fwd.hh>
#include <seating_optimization/seating_problem_elements/constraints/Constraint.fwd.hh>
#include <seating_optimization/seating_problem_elements/restraints/Restraint.fwd.hh>
#include <seating_optimization/seating_problem/SeatingSolution.fwd.hh>

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

	/// @brief Get the number of guests.
	masala::base::Size n_guests() const;

	/// @brief Get the number of seats.
	masala::base::Size n_seats() const;

	/// @brief Get the number of tables.
	masala::base::Size n_tables() const;

	/// @brief Access a guest, by guest index.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP
	guest(
		masala::base::Size const guest_index
	) const;

	/// @brief Access a seat, by seat index.
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP
	seat(
		masala::base::Size const seat_index
	) const;

	/// @brief Has this object been finalized?
	bool finalized() const;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Configure from the lines of a file.
	void
	configure_from_problem_definition_file_lines(
		std::vector< std::string > const & file_lines
	);

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

	/// @brief Get a vector of pairs of global seat indices, with one pair for every two seats that are next to one another
	/// at each table in the problem.
	std::vector< std::pair< masala::base::Size, masala::base::Size > >
	get_adjacent_seat_global_indices() const;

	/// @brief Configure and finalize a cost function network optimization problem from this object.
	/// @param[in] problem A shared pointer to an empty problem.  Filled and finalized by this operation.
	void
	set_up_cfn_problem(
		masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationProblem_API & problem,
		std::vector< std::vector< bool > > & allowed_seats,
		std::vector< std::map< masala::base::Size,  masala::base::Size > > & guest_choice_to_seat_index,
		std::vector< std::map<  masala::base::Size,  masala::base::Size > > & seat_index_to_guest_choice
	) const;

	/// @brief Given a CFN solution, generate a SeatingSolution object from it.
	/// @note The returned object is unfinalized, since it needs a shared pointer from this SeatingProblem object to be cached in it.
	SeatingSolutionSP
	seating_solution_from_cfn_solution(
		masala::numeric_api::auto_generated_api::optimization::cost_function_network::CostFunctionNetworkOptimizationSolution_API const & cfn_solution,
		std::vector< std::map< masala::base::Size, masala::base::Size > > const & guest_choice_to_seat_index
	) const;

	/// @brief Given a global seat index, determine whether this seat is at a table.
	bool seat_is_at_a_table( masala::base::Size const seat_index ) const;

	/// @brief Get a vector of global seat indices for the seats at a given table.
	/// @param[in] table_index The index of the table.
	std::vector< masala::base::Size > seats_at_table( masala::base::Size const table_index ) const;

	/// @brief Given a global seat index, determine the table index and local index of the seat at the table.  Throws if the seat isn't at a table.
	std::pair< masala::base::Size, masala::base::Size > table_and_local_seat_index_from_global_seat_index( masala::base::Size const seat_index ) const;

	/// @brief Indicate that this object is fully set up.
	void finalize();

	/// @brief Print the problem to the tracer.  Problem must be finalized, or this function throws.
	void print_problem() const;

	/// @brief Print the problem to a string.  Problem must be finalized, or this function throws.
	std::string get_problem_string() const;

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

	/// @brief Print the problem to a string.  Problem must be finalized, or this function throws.
	/// @details This should be called from a mutex-locked context only.
	std::string protected_print_problem_to_string() const;

	/// @brief Given a global seat index, determine whether this seat is at a table.
	/// @details Intended to be called from a mutex-locked context.
	bool protected_seat_is_at_a_table( masala::base::Size const seat_index ) const;

	/// @brief Given a global seat index, determine the table index and local index of the seat at the table.  Throws if the seat isn't at a table.
	/// @details Intended to be called from a mutex-locked context.
	std::pair< masala::base::Size, masala::base::Size > protected_table_and_local_seat_index_from_global_seat_index( masala::base::Size const seat_index ) const;

	/// @brief Add a guest.  Stored directly; not cloned.  Throws if the unique guest ID has already been taken.
	/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
	void
	protected_add_guest(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP const & guest_in
	);

	/// @brief Add a table.  Stored directly; not cloned.
	/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
	void
	protected_add_table(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP const & table_in
	);

	/// @brief Add a room.  Primarily for visualization
	/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
	void
	protected_add_room(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::RoomCSP const & room_in
	);

	/// @brief Add a loose seat.  Stored directly; not cloned.  (NOT YET SUPPORTED -- THROWS.)
	/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
	void
	protected_add_seat(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP const & seat_in
	);

	/// @brief Add a constraint.  Stored directly; not cloned.
	/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
	void
	protected_add_constraint(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::ConstraintCSP const & constraint_in
	);

	/// @brief Add a restraint.  Stored directly; not cloned.
	/// @note This version performs no mutex locking.  It should be called from a mutex-locked context.
	void
	protected_add_restraint(
		seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::RestraintCSP const & restraint_in
	);

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

	/// @brief Has this object been finalized?
	bool finalized_ = false;

	/// @brief The guests that we are storing, stored by unique identifier.
	/// @details The UID string points to a pair of guest index, guest object.
	std::map< std::string, std::pair< masala::base::Size, seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP > > guests_;

	/// @brief The guests, stored by guest index.  Populated at finalization time.
	std::map< masala::base::Size, seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::GuestCSP > guests_by_index_;

	/// @brief The tables, stored by index.
	std::vector< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::TableCSP > tables_;

	/// @brief The rooms, stored by index.  Primarily for visualization
	std::vector< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::RoomCSP > rooms_;

	/// @brief The zero-based absolute indices of the seats.
	std::map< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP, masala::base::Size > seat_indices_;

	/// @brief The seats indexed by their zero-based absolute indices.  Populated at finalization time.
	std::map< masala::base::Size, seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatCSP > seats_by_index_;

	/// @brief The constraints, sorted by index.
	std::vector< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::constraints::ConstraintCSP > constraints_;

	/// @brief The restraints, sorted by index.
	std::vector< seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::restraints::RestraintCSP > restraints_;

}; // class SeatingProblem

} // namespace seating_problem
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_SeatingProblem_hh