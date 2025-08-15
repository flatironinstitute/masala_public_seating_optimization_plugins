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

/// @file src/seating_optimization/api/generate_api_classes.hh
/// @brief Utility functions to generate all of the classes in seating_optimization that define
/// an API.  Used for auto-generating the seating_optimization api.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_api_generate_api_classes_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_api_generate_api_classes_hh

// Base headers
#include <base/MasalaObject.fwd.hh>

// STL headers
#include <vector>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace api {

    /// @brief Generate an instance of each of the classes in seating_optimization that have a defined API, and
    /// return a vector of owning pointers to these instances.
    /// @details Used for auto-generating the seating_optimization API description (as a JSON file).
    std::vector< masala::base::MasalaObjectSP > generate_api_classes();

} // namespace api
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_api_generate_api_classes_hh