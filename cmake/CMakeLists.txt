# Seating Optimization Masala Plugins 
# Copyright (C) 2025 Vikram K. Mulligan
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

## This file actually does the building.

CMAKE_MINIMUM_REQUIRED( VERSION 3.10 )

# Set project attributes:
PROJECT( MasalaSeatingOptimizationPlugins VERSION 0.1 DESCRIPTION "Seating optimization plugins and applications that use Masala optimizers." )

# Default build is release mode.  To build in debug mode (recommeded for development),
# change the nex tline to SET( MODE debug ).
SET( MODE release )
#SET( MODE debug )
IF( ${MODE} STREQUAL "release" )
	SET( CMAKE_BUILD_TYPE Release )
ELSEIF( ${MODE} STREQUAL "debug" )
	SET( CMAKE_BUILD_TYPE Debug )
ELSE()
	MESSAGE( FATAL_ERROR "The only allowed compilation modes are debug and release." )
ENDIF()

# Uncomment the following to enable compilation with support for AVX512 instructions on Linux or Windows systems.
# Note that this only has an effect in relase mode.
#SET( MASALA_SUPPORT_AVX512 TRUE )

# Uncomment the following to enable deprecated API functions.
#ADD_DEFINITIONS( -DMASALA_ENABLE_DEPRECATED_FUNCTIONS )

# Uncomment the following to disable deprecation warnings.
#ADD_DEFINITIONS( -DMASALA_DISABLE_DEPRECATION_WARNINGS )

# Ensure that version is set in only one place, in the PROJECT line at the top of this file:
ADD_DEFINITIONS( -DMASALA_QUANTUM_COMPUTING_PLUGINS_MAJOR_VERSION=${CMAKE_PROJECT_VERSION_MAJOR} -DMASALA_QUANTUM_COMPUTING_PLUGINS_MINOR_VERSION=${CMAKE_PROJECT_VERSION_MINOR} )

SET( LINK_TYPE SHARED )
SET( LINK_EXTERNAL_LIBS stdc++ )
SET( CMAKE_MACOSX_RPATH 0 )
SET( CMAKE_SKIP_BUILD_RPATH FALSE )
SET( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
SET( THREADS_PREFER_PTHREAD_FLAG ON )
SET( Catch2_DIR ../catch2_build )
SET( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
#SET(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -pedantic-errors")
LINK_DIRECTORIES( ../lib )
FIND_PACKAGE( Threads REQUIRED )

# Set header file locations:
INCLUDE_DIRECTORIES( SYSTEM ../ ../src/ ../headers/masala/headers/ )

# Compiler settings and compilation flags:
INCLUDE( compiler.cmake )

# Generate the build lists for the seating_optimization library and its API library:
execute_process(
    COMMAND python3 ../code_templates/generate_cmake_build.py quantum_computing_masala_plugins ${CMAKE_PROJECT_VERSION_MAJOR} ${CMAKE_PROJECT_VERSION_MINOR} seating_optimization ../src/seating_optimization cmake_generated/seating_optimization.cmake cmake_generated/seating_optimization_api.cmake cmake_generated/seating_optimization_tests.cmake
)

# Generate the build lists for the registration library and its API library:
execute_process(
    COMMAND python3 ../code_templates/generate_cmake_build.py quantum_computing_masala_plugins ${CMAKE_PROJECT_VERSION_MAJOR} ${CMAKE_PROJECT_VERSION_MINOR} registration ../src/registration cmake_generated/registration.cmake cmake_generated/registration_api.cmake cmake_generated/registration_tests.cmake
)

# Generate the build lists for the applications library and its API library:
execute_process(
    COMMAND python3 ../code_templates/generate_cmake_build.py quantum_computing_masala_plugins ${CMAKE_PROJECT_VERSION_MAJOR} ${CMAKE_PROJECT_VERSION_MINOR} applications ../src/applications cmake_generated/applications.cmake NONE NONE
)

# Build the seating_optimization library:
INCLUDE( ../build/cmake_generated/seating_optimization.cmake )

# Build the seating_optimization API library:
INCLUDE( ../build/cmake_generated/seating_optimization_api.cmake )

# Build the registration library:
INCLUDE( ../build/cmake_generated/registration.cmake )

# Build the registration_api library:
INCLUDE( ../build/cmake_generated/registration_api.cmake )

# Build the applications library:
INCLUDE( ../build/cmake_generated/applications.cmake )


# Build tests:

# Build tests for the seating_optimization library.
INCLUDE( ../build/cmake_generated/seating_optimization_tests.cmake )

# Build tests for the registration library.
INCLUDE( ../build/cmake_generated/registration_tests.cmake )
