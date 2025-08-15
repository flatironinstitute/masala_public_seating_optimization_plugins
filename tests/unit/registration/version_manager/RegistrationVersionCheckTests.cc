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

/// @file tests/unit/registration/version_manager/RegistrationVersionCheckTests.cc
/// @brief Unit tests for registering the seating optimization Masala plugins library, and for ensuring that
/// its version requirements are satisfied.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Unit testing library (Catch2) headers:
#include <external/catch2/single_include/catch2/catch.hpp>

// Unit headers:
#include <base/managers/environment/MasalaEnvironmentManager.hh>
#include <base/managers/version/MasalaVersionManager.hh>
#include <base/managers/plugin_module/MasalaPluginModuleManager.hh>
#include <base/managers/plugin_module/MasalaPluginLibraryManager.hh>
#include <registration_api/register_library.hh>

// Uncomment the following once we have at least one plugin in this library:
//#define AT_LEAST_ONE_PLUGIN_DEFINED

namespace seating_optimization_masala_plugins {
namespace tests {
namespace unit {
namespace registration {
namespace version_manager {

TEST_CASE( "Register and check version compatibility", "[seating_optimization_masala_plugins::registration_api::register_library][registration][MasalaVersionManager]" ) {
    using namespace masala::base::managers::version;
    signed long int n_before, n_registered, n_after;
    REQUIRE_NOTHROW([&](){
        MasalaVersionManagerHandle vm( MasalaVersionManager::get_instance() );
        n_before = vm->n_modules_registered();
        seating_optimization_masala_plugins::registration_api::register_library();
        n_registered = vm->n_modules_registered();
        seating_optimization_masala_plugins::registration_api::unregister_library();
        n_after = vm->n_modules_registered();
    }() );

    CHECK( n_before == 1 );
    CHECK( n_registered == 2 );
    CHECK( n_after == 1 );
}

TEST_CASE( "Register and check version compatibility using MasalaLibraryManager", "[seating_optimization_masala_plugins::registration_api::register_library][registration][MasalaEnvironmentManager][MasalaPluginLibraryManager][MasalaVersionManager]" ) {
    using namespace masala::base::managers::environment;
    using namespace masala::base::managers::version;
    using namespace masala::base::managers::plugin_module;

    signed long int n_before, n_registered, n_after;
    REQUIRE_NOTHROW([&](){
        MasalaEnvironmentManagerHandle envman( MasalaEnvironmentManager::get_instance() );
        std::string libpath;
        CHECK_OR_THROW(
            envman->get_environment_variable( "MASALA_SEATING_OPTIMIZATION_PLUGINS", libpath ),
            "seating_optimization_masala_plugins::tests::unit::registration::version_manager::RegistrationVersionCheckTests",
            "Register_and_check_version_compatibility_using_MasalaLibraryManager",
            "The MASALA_SEATING_OPTIMIZATION_PLUGINS environment variable must point to the directory "
            "of the seating optimization Masala plugins repository for this test."
        );
        MasalaPluginLibraryManagerHandle libman( MasalaPluginLibraryManager::get_instance() );
    
        MasalaVersionManagerHandle vm( MasalaVersionManager::get_instance() );
        n_before = vm->n_modules_registered();
        libman->load_and_register_plugin_libraries_in_subdirectories( libpath, true );
        n_registered = vm->n_modules_registered();
        seating_optimization_masala_plugins::registration_api::unregister_library();
        n_after = vm->n_modules_registered();
    }() );

    CHECK( n_before == 1 );
    CHECK( n_registered == 2 );
    CHECK( n_after == 1 );
}

TEST_CASE( "Register and check that plugins were registered", "[seating_optimization_masala_plugins::registration_api::register_library][registration][MasalaPluginModuleManager]" ) {
    using namespace masala::base::managers::plugin_module;
    signed long int n_plugins_before, n_plugins_registered, n_plugins_after;
    REQUIRE_NOTHROW([&](){
        MasalaPluginModuleManagerHandle pm( MasalaPluginModuleManager::get_instance() );
        n_plugins_before = pm->get_all_plugin_list().size();
        seating_optimization_masala_plugins::registration_api::register_library();
        n_plugins_registered = pm->get_all_plugin_list().size();
        seating_optimization_masala_plugins::registration_api::unregister_library();
        n_plugins_after = pm->get_all_plugin_list().size();
    }() );

#ifdef AT_LEAST_ONE_PLUGIN_DEFINED
    CHECK( n_plugins_registered > n_plugins_before );
    CHECK( n_plugins_registered - n_plugins_before >= 4 ); // This library defines at least 4 plugins.
#endif
    CHECK( n_plugins_after == n_plugins_before );
}

TEST_CASE( "Register and check that plugins were registered using MasalaLibraryManager", "[seating_optimization_masala_plugins::registration_api::register_library][registration][MasalaEnvironmentManager][MasalaPluginLibraryManager][MasalaPluginModuleManager]" ) {
    using namespace masala::base::managers::environment;
    using namespace masala::base::managers::plugin_module;

    signed long int n_plugins_before, n_plugins_registered, n_plugins_after;
    REQUIRE_NOTHROW([&](){
        MasalaEnvironmentManagerHandle envman( MasalaEnvironmentManager::get_instance() );
        std::string libpath;
        CHECK_OR_THROW(
            envman->get_environment_variable( "MASALA_SEATING_OPTIMIZATION_PLUGINS", libpath ),
            "seating_optimization_masala_plugins::tests::unit::registration::version_manager::RegistrationVersionCheckTests",
            "Register_and_check_version_compatibility_using_MasalaLibraryManager",
            "The MASALA_SEATING_OPTIMIZATION_PLUGINS environment variable must point to the directory "
            "of the seating optimization Masala plugins repository for this test."
        );
        MasalaPluginLibraryManagerHandle libman( MasalaPluginLibraryManager::get_instance() );

        MasalaPluginModuleManagerHandle pm( MasalaPluginModuleManager::get_instance() );
        n_plugins_before = pm->get_all_plugin_list().size();
        libman->load_and_register_plugin_libraries_in_subdirectories( libpath, true );
        n_plugins_registered = pm->get_all_plugin_list().size();
        seating_optimization_masala_plugins::registration_api::unregister_library();
        n_plugins_after = pm->get_all_plugin_list().size();
    }() );

#ifdef AT_LEAST_ONE_PLUGIN_DEFINED
    CHECK( n_plugins_registered > n_plugins_before );
    CHECK( n_plugins_registered - n_plugins_before >= 4 ); // This library defines at least 4 plugins.
#endif
    CHECK( n_plugins_after == n_plugins_before );
}


} // namespace version_manager
} // namespace registration
} // namespace unit
} // namespace tests
} // namespace seating_optimization_masala_plugins
