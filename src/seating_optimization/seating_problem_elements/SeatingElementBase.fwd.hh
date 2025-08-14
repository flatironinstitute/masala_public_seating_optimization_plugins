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

/// @file src/seating_optimization/seating_problem_elements/SeatingElementBase.fwd.hh
/// @brief Forward declarations for a SeatingElementBase.
/// @details The SeatingElementBase base class is a common base class for all seating elements.  It is
/// not meant to be instantiated outside of the API definition system.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_SeatingElementBase_fwd_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_SeatingElementBase_fwd_hh

#include <base/managers/memory/util.hh> // For MASALA_SHARED_POINTER

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

	class SeatingElementBase;

	/// @brief We will use the convention that an class name followed by SP
	/// represents a MASALA_SHARED_POINTER for objects of that class.
	using SeatingElementBaseSP = MASALA_SHARED_POINTER< SeatingElementBase >;

	/// @brief We will use the convention that an class name followed by CSP
	/// represents a MASALA_SHARED_POINTER for const objects of that class.
	using SeatingElementBaseCSP = MASALA_SHARED_POINTER< SeatingElementBase const >;

	/// @brief We will use the convention that an class name followed by WP
	/// represents a MASALA_WEAK_POINTER for objects of that class.
	using SeatingElementBaseWP = MASALA_WEAK_POINTER< SeatingElementBase >;

	/// @brief We will use the convention that an class name followed by CWP
	/// represents a MASALA_WEAK_POINTER for const objects of that class.
	using SeatingElementBaseCWP = MASALA_WEAK_POINTER< SeatingElementBase const >;

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif //Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_SeatingElementBase_fwd_hh