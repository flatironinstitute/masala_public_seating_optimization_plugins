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

/// @file src/registration_api/register_library.cc
/// @brief Function to register a library with the Masala base plugin manager.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#include <registration_api/register_library.hh>
#include <registration/register_sub_libraries.hh>

#include <base/managers/tracer/MasalaTracerManager.hh>
#include <base/managers/version/MasalaModuleVersionInfo.hh>
#include <base/managers/version/MasalaVersionManager.hh>

namespace seating_optimization_masala_plugins {
namespace registration_api {

/// @brief Register this library with the Masala base plugin manager.
extern "C"
void
register_library() {
    using namespace masala::base::managers::version;
    using masala::base::Size;
    masala::base::managers::tracer::MasalaTracerManagerHandle tm( masala::base::managers::tracer::MasalaTracerManager::get_instance() );
    tm->write_to_tracer(
        "seating_optimization_masala_plugins::registration_api::register_library", "Registering seating optimization Masala plugins."
    );
    tm->add_destruction_message(
        "SEATING_OPTIMIZATION_MASALA_PLUGINS",
        {  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
            "This run of Masala used the Seating        ",
            "Optimization Masala Plugins library.  This ",
            "library's citation is: K.A. Nicholas and   ",
            "V.K. Mulligan.  (2026).  Entangled happily ",
            "ever after: Wedding reception seating      ",
            "mapped to classical and quantum optimizers.",
            "Manuscript under review.    Preprint       ",
            "available from \033[4marXiv\033[24m:                      ",
            "https://arxiv.org/abs/2604.10497.          ",
            "                                           "
        },
        "seating_optimization_masala_plugins_citation_message"
    );

    MasalaModuleVersionInfoSP module_version_info(
        masala::make_shared< MasalaModuleVersionInfo >(
            "Seating Optimization Masala Plugins",
            std::pair< Size, Size >( MASALA_SEATING_OPTIMIZATION_PLUGINS_MAJOR_VERSION, MASALA_SEATING_OPTIMIZATION_PLUGINS_MINOR_VERSION )
        )
    );
    module_version_info->add_requirement_with_minimum_version(
        "Masala",
        true,
        std::pair< Size, Size >( 1, 8 ), // Min version
        "",
        "Version 1.8 was the version tested against for the public release.  "
        "Version 0.15 added the add_to_onebody_penalty() and add_to_twobody_penalty() functions for pairwise precomputed CFN problems.  "
        "Version 0.14 added the protected_make_independent() function for CFN optimizers.  "
        "Version 0.13 updated some of the CMake files, made the release build the default build, and added support for AVX512 instructions.  "
        "Version 0.12 updated the Masala random generator to ensure that different MPI processes had different random seeds.  "
        "Version 0.11 changed base class names for optimizers, for consistency.  "
        "Version 0.10 added support for deprecation annotations, and updated the build system for centralized versioning.  "
        "Version 0.9 added support for no-UI function annotations.  "
        "Version 0.8 cleaned up some details related to cost function problem scratch spaces that didn't conform to Masala coding conventions.  "
		"Prior to version 0.7, cost function problems and cost functions could not accept scratch space pointers.  "
		"Prior to version 0.6, annotations for API functions indicating preferred data representations were not available.  "
        "Prior to version 0.5, there were some issues preventing implementation of gradient-descent minimizers "
        "in the standard Masala plugins library (which is commonly used in conjunction with this library).  "
        "Prior to version 0.4, reset() and finalize() functions in the "
		"cost function network optimization problem class were unnecessarily virtual.  "
        "Prior to version 0.3, accessor functions for setter descriptions in API definition objects returned the wrong object type, "
		"requiring unnecessary dynamic casting."
    );
    module_version_info->add_requirement_with_minimum_version(
        "Standard Masala Plugins",
        false,
        std::pair< Size, Size >( 1, 2 ), // Min version
        "",
        "Version 1.2 was the version tested against for the public release.  "
        "Version 0.15 added the add_to_onebody_penalty() and add_to_twobody_penalty() functions for derived classes of pairwise precomputed CFN problems.  "
        "Version 0.14 added the protected_make_independent() function for CFN optimizers.  "
        "Version 0.13 corrected the categories for real-valued local (RVL) optimizer engines.  "
        "Version 0.12 updated CMake files, made the release build (rather than the debug build) the default build, and added support for AVX512 instructions.  "
        "Version 0.11 changed base class names for optimizers, for consistency.  "
        "Version 0.10 added support for deprecation annotations, and updated the build system for centralized versioning.  "
        "Version 0.9 added no-UI function annotations.  "
        "Version 0.8 cleaned up some details related to cost function problem scratch spaces that didn't conform to Masala coding conventions.  "
		"Prior to version 0.7, cost function problems and cost functions could not accept scratch space pointers.  "
		"Prior to version 0.6, annotations for API functions indicating preferred data representations were not available.  "
        "Version 0.5 adds gradient-descent minimizers which are intended for use with the approximate binary QUBO encoding.  "
        "Version 0.4 fixes the cost function network optimization problem API.  "
        "Prior to version 0.3, the BinaryCostFunctionNetworkProblemRosettaFileInterpreter did not exist."
    );

    module_version_info->add_requirement_with_minimum_version(
        "Quantum Computing Masala Plugins",
        false,
        std::pair< Size, Size >( 0, 16 ), // Min version
        "",
        "Version 0.16 added the add_to_onebody_penalty() and add_to_twobody_penalty() functions for derived classes of pairwise precomputed CFN problems.  "
    );
    MasalaVersionManager::get_instance()->add_library_information( module_version_info );
    seating_optimization_masala_plugins::registration::register_sub_libraries();
}

/// @brief Unregister this library with the Masala base plugin manager.
extern "C"
void
unregister_library() {
    using namespace masala::base::managers::version;
    masala::base::managers::tracer::MasalaTracerManager::get_instance()->write_to_tracer(
        "seating_optimization_masala_plugins::registration_api::register_library", "Unregistering seating optimization Masala plugins."
    );
    MasalaVersionManager::get_instance()->remove_library_information( "Seating Optimization Masala Plugins" );
    seating_optimization_masala_plugins::registration::unregister_sub_libraries();
}

} // namespace registration_api
} // namespace seating_optimization_masala_plugins
