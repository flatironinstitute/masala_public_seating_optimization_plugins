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

/// @file src/registration_api/register_library.hh
/// @brief Function to register a library with the Masala base plugin manager.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_registration_api_register_library_hh
#define Seating_Optimization_Masala_Plugins_src_registration_api_register_library_hh

namespace seating_optimization_masala_plugins {
namespace registration_api {

    /// @brief Register this library with the Masala base plugin manager.
    extern "C" void register_library();

    /// @brief Unregister this library with the Masala base plugin manager.
    extern "C" void unregister_library();

} // namespace registration_api
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_registration_api_register_library_hh
