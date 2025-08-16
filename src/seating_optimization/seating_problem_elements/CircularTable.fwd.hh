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

/// @file src/seating_optimization/seating_problem_elements/CircularTable.fwd.hh
/// @brief Forward declarations for a CircularTable.
/// @details As the name suggests, a circular table is a table with a circular shape and chairs arranged around it.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_CircularTable_fwd_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_CircularTable_fwd_hh

#include <base/managers/memory/util.hh> // For MASALA_SHARED_POINTER

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

	class CircularTable;

	/// @brief We will use the convention that an class name followed by SP
	/// represents a MASALA_SHARED_POINTER for objects of that class.
	using CircularTableSP = MASALA_SHARED_POINTER< CircularTable >;

	/// @brief We will use the convention that an class name followed by CSP
	/// represents a MASALA_SHARED_POINTER for const objects of that class.
	using CircularTableCSP = MASALA_SHARED_POINTER< CircularTable const >;

	/// @brief We will use the convention that an class name followed by WP
	/// represents a MASALA_WEAK_POINTER for objects of that class.
	using CircularTableWP = MASALA_WEAK_POINTER< CircularTable >;

	/// @brief We will use the convention that an class name followed by CWP
	/// represents a MASALA_WEAK_POINTER for const objects of that class.
	using CircularTableCWP = MASALA_WEAK_POINTER< CircularTable const >;

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif //Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_CircularTable_fwd_hh