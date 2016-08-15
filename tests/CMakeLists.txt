################################################################################
#
# Top-level test CMakeLists.txt
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

project(poly2tri_test VERSION 0.3.3 LANGUAGES CXX) # match poly2tri library version

################################################################################
# External dependencies
if(NOT TARGET poly2tri)
  find_package(poly2tri CONFIG REQUIRED)
  if(poly2tri_FOUND)
    log_message(${verbose} "Found poly2tri: ${poly2tri_DIR}")
  endif()
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 3.1 REQUIRED)
if(glfw3_FOUND)
  log_message(${verbose} "Found glfw: ${glfw3_DIR}")
endif()

################################################################################
# Tests

# Data install directory
if (WIN32 AND NOT CYGWIN)
  set(data_install_dir share)
else()
  set(data_install_dir share/poly2tri)
endif()

# Install data
install(DIRECTORY
  data
  DESTINATION ${data_install_dir}
  COMPONENT RUNTIME
  FILES_MATCHING REGEX ".*\\.(dat|dae)$"
)

### Test A
set(target p2t)
set(src main.cc)
add_executable(${target} ${src})

target_include_directories(${target} PRIVATE 
  ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(${target} PRIVATE
  poly2tri
  glfw
  ${OPENGL_gl_LIBRARY}
)
#
# CMAKE_<CONFIG>_POSTFIX has no effect on executable,
# issue http://www.cmake.org/Bug/view.php?id=7868
set_property(TARGET ${target} PROPERTY DEBUG_POSTFIX "d")

#FIXME how to add correct path to data for build and install
add_test(NAME test_${target} COMMAND ${target} ${CMAKE_CURRENT_SOURCE_DIR}/data/dude.dat 350 500 2)

install(TARGETS ${target}
  DESTINATION bin
)

### Additional tests go here...