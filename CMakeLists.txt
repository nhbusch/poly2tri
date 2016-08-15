################################################################################
#
# Top-level CMakeLists.txt
#
# Copyright (c) 2016 Nils H. Busch. All rights reserved.
#
# Distributed under the MIT License (MIT).
# See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT.
#
################################################################################

################################################################################
# Preamble
cmake_minimum_required(VERSION 3.0)

foreach(p
  CMP0051 # CMake 3.1
  CMP0052 # CMake 3.1
  CMP0053 # CMake 3.1
  CMP0054 # CMake 3.1
  CMP0056 # CMake 3.2
  CMP0058 # CMake 3.3
  CMP0064 # CMake 3.4
  )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

# Set PROJECT_NAME, PROJECT_VERSION
project(poly2tri VERSION 0.3.3 LANGUAGES CXX)
string(TOUPPER ${PROJECT_NAME} project_name_capitalized)

# Append CMake module search path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Enforce out-of-source build
include(EnforceOutOfSourceBuild)
enforce_out_of_source_build()

# Logging
include(LogMessage)
set(verbose ON)

################################################################################
# Extension(s)
include(CMakeDependentOption)

# Support for export macros
include(GenerateExportHeader)

# Support config-file package
include(CMakePackageConfigHelpers)

################################################################################
# Option(s)
option(BUILD_SHARED_LIBS "Build as shared libraries." OFF)

option(${project_name_capitalized}_BUILD_TESTS "Build tests." OFF)

option(${project_name_capitalized}_BUILD_BENCHES "Build benchmarks." OFF)

option(${project_name_capitalized}_BUILD_EXAMPLES "Build examples." OFF)

option(${project_name_capitalized}_BUILD_DOCUMENTATION "Build documentation." OFF)
if(${project_name_capitalized}_BUILD_DOCUMENTATION)
  # Possibly add dependent option for building html, pdf...
  add_subdirectory(doc)
endif()

# Additional options go here...

################################################################################
# External dependencies


################################################################################
# Target(s)

# Include source and build directories during build
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

### poly2tri library
set(target poly2tri)
set(srcs
  src/common/shapes.cc
  src/sweep/advancing_front.cc
  src/sweep/cdt.cc
  src/sweep/sweep.cc
  src/sweep/sweep_context.cc
)

set(incs
  include/poly2tri/poly2tri.h
  include/poly2tri/common/shapes.h
  include/poly2tri/common/utils.h
  include/poly2tri/sweep/advancing_front.h
  include/poly2tri/sweep/cdt.h
  include/poly2tri/sweep/sweep.h
  include/poly2tri/sweep/sweep_context.h
)

# Library target
add_library(${target} ${srcs})

# Target definitions

# By default, export DEBUG configuration value
# {Debug: -DFOO_BAR_DEBUG=1, Release/other: -DFOO_BAR_DEBUG=0}
string(TOUPPER ${target} target_capitalized)
target_compile_definitions(${target} PUBLIC
  "${project_name_capitalized}_${target_capitalized}_DEBUG=$<CONFIG:Debug>" # or $<$<CONFIG:DEBUG>:FOO_DEBUG=1
)

# Limit compiler warnings from standard libs
target_compile_definitions(${target} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

# Target compile options

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  set(cxx_release_flags -O3 -ffast-math)
elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
 set(cxx_release_flags /Ox /fp:fast)              # FIXME fast also for debug?
endif()
target_compile_options(${target}
  PRIVATE
  $<$<CONFIG:Release>:${cxx_release_flags}>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4715>
  $<$<CXX_COMPILER_ID:MSVC>:/WX>                  # treat warnings as  errors
)

# Generate export macros for library target
generate_export_header(${target})

# Other properties...
#set_property(TARGET ... PROPERTY ...)

# Add debug postfix to disambiguate release and debug targets
# This could also be initialized globally by the appropriate CMake variable
#set(CMAKE_DEBUG_POSTFIX d)
set_property(TARGET ${target} PROPERTY DEBUG_POSTFIX d)

# Include directories requirements
# Include by '#include <bar/some_header.hpp>'
target_include_directories(${target} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # build tree
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> # build tree
  $<INSTALL_INTERFACE:include/${target}>                 # install tree
)
#log_message(${verbose} "Includes (build-tree): $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
#log_message(${verbose} "                       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")
#log_message(${verbose} "Includes (install):    $<INSTALL_INTERFACE:include/${target}>")

target_include_directories(${target} PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${target}>)

# Link and propagate the dependencies
#target_link_libraries(${target} INTERFACE ...)

###
# Additional targets go here...

################################################################################
# Package configuration (for consumers of the target)

### Layout
#   * <prefix>/lib/
#   * <prefix>/include/
#   * <prefix>/lib/cmake/${PROJECT-NAME} for *.nix
#   * <prefix>/CMake/ maps to <prefix>/${PROJECT_NAME}/CMake on Windows
# Workaround for issue https://gitlab.kitware.com/cmake/cmake/issues/16212
if (WIN32 AND NOT CYGWIN)
  set(config_install_dir cmake)
else()
  set(config_install_dir lib/cmake/${PROJECT_NAME})
endif()

set(include_install_dir include)

set(_generated_dir "${CMAKE_CURRENT_BINARY_DIR}")
set(version_config_file "${_generated_dir}/${PROJECT_NAME}-config-version.cmake")
set(project_config_file "${_generated_dir}/${PROJECT_NAME}-config.cmake")
set(targets_export "${PROJECT_NAME}-targets")
set(namespace "${PROJECT_NAME}::")

# Alias
add_library(${namespace}${target} ALIAS ${target})

# Version support
# Note: PROJECT_VERSION is used for VERSION (since 3.0)
write_basic_package_version_file("${version_config_file}"
  COMPATIBILITY SameMajorVersion # Semver, generally <AnyNewerVersion|SameMajorVersion|ExactVersion>
)

# Write package config file
configure_package_config_file(
  "cmake/config.cmake.in"
  "${project_config_file}"
  INSTALL_DESTINATION "${config_install_dir}"
)

################################################################################
# Installation

# Targets
install(TARGETS ${target}
  EXPORT ${targets_export}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION ${include_install_dir}
)


# Headers
#   * include/${target}/.*\\.(hpp|h|inl)$ -> <prefix>/include/${target}
install(DIRECTORY
  include/${target}
  DESTINATION ${include_install_dir}
  COMPONENT Development
  FILES_MATCHING REGEX ".*\\.(hpp|h|inl)$"
)

# Export headers
#   * ${CMAKE_CURRENT_BINARY_DIR}/${target}_export.h -> <prefix>/include
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${target}_export.h"
  DESTINATION ${include_install_dir}
  COMPONENT Development
)

# Package & version config
install(FILES
  ${project_config_file}
  ${version_config_file}
  DESTINATION ${config_install_dir}
)

# Import target
install(EXPORT ${targets_export}
  NAMESPACE ${namespace}
  DESTINATION ${config_install_dir}
)

# Import target for build tree (for targets within same build)
# Use by pointing CMake to build_dir 'cmake -DCMAKE_PREFIX_PATH=<build_dir> ...'
export(EXPORT ${targets_export}
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${targets_export}.cmake"
  NAMESPACE ${namespace}
)

################################################################################
# Sub-projects

if(${project_name_capitalized}_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(${project_name_capitalized}_BUILD_BENCHES)
  add_subdirectory(benches EXCLUDE_FROM_ALL)
endif()

if(${project_name_capitalized}_BUILD_EXAMPLES)
  add_subdirectory(examples EXCLUDE_FROM_ALL)
endif()