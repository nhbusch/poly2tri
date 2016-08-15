################################################################################
#
# LogMessage.cmake
#
# Copyright (c) 2016 Nils H. Busch. All rights reserved.
#
# Distributed under the MIT License (MIT).
# See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT.
#
################################################################################

#.rst:
# LogMessage.cmake
#-------------------------
#
# .. command:: log_message
#
# Simple conditional logging of incidental information::
#
#   log_message(<enable> <msg>)
#
# Parameters
# ^^^^^^^^^^
#   ``enable``    If true, message is logged
#   ``msg``       Log message
#
function(log_message enable msg)
  if(enable AND NOT ${msg} STREQUAL "")
    message(STATUS ${msg})
  endif()
endfunction()