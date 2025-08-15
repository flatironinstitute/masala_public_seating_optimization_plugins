/*
    Seating Optimization Masala Plugins
    Copyright (C) 2022 Vikram K. Mulligan

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

/// @file src/registration/registration_api/generate_registration_api.hh
/// @brief An application used for auto-generating the registration API JSON.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

// Core headers
#include <base/types.hh>

// Registration headers

// Base headers
#include <base/MasalaObject.hh>
#include <base/api/MasalaObjectAPIDefinition.hh>
#include <base/managers/disk/MasalaDiskManager.hh>

// External headers
#include <external/nlohmann_json/single_include/nlohmann/json.hpp>

// STL headers
#include <sstream>

// Program entry point:
int
main(
    // int argc,
    // char * argv[]
    int, char**
) {
    using namespace masala::base::managers::plugin_module;
    nlohmann::json api_definition;
    api_definition["FileType"] = "API_definition";
    api_definition["Module"] = "Registration";
    nlohmann::json api_entries;
    
    // std::vector< masala::base::MasalaObjectSP > const api_objects( seating_optimization_masala_plugins::seating_optimization::api::generate_api_classes() );

    // for( auto const & api_object : api_objects ) {
    //     masala::base::api::MasalaObjectAPIDefinitionCSP api_def( api_object->get_api_definition() );
    //     api_entries[ api_object->class_namespace() + "::" + api_object->class_name() ] = *api_def->get_json_description();
    // }
    api_definition["Elements"] = api_entries;

    std::ostringstream ss;
    ss << api_definition.dump( 1, '\t' ) << "\n";
    masala::base::managers::disk::MasalaDiskManager::get_instance()->write_ascii_file( "registration_api.json", ss.str() );

    return 0;
}