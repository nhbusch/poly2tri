################################################################################
#
# EnforceOutOfSourceBuild.cmake
#
# Copyright (c) 2016 Nils H. Busch. All rights reserved.
#
# Distributed under the MIT License (MIT).
# See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT.
#
################################################################################

#.rst:
# EnforceOutOfSourceBuild.cmake
#------------------------------
#
# .. command:: enforce_out_of_source_build
#
# Enforce out-of-source-build::
#
#   enforce_out_of_source_build()
# 
# Warns if CMAKE_BINARY_DIR equals CMAKE_SOURCE_DIR
# There is currently no way to prevent CMake from generating files in source dir
# prior to this check, see also issue #14818.

function(enforce_out_of_source_build)
  # make sure the user doesn't play dirty with symlinks
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

  # disallow in-source builds
  if("${srcdir}" STREQUAL "${bindir}")
    message("*** WARNING: You are attempting to build in the source directory,")
    message("*** WARNING: which is considered bad practice.")
    message("*** WARNING: Instead, you should create a dedicated build directory ")
    message("*** WARNING: and run 'cmake /path/to/${PROJECT_NAME} [options]' from there.")
    message("")
    message(FATAL_ERROR "*** Aborting. ***")
  endif()
endfunction()