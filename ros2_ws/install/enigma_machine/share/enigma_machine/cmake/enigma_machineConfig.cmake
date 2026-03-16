# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_enigma_machine_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED enigma_machine_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(enigma_machine_FOUND FALSE)
  elseif(NOT enigma_machine_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(enigma_machine_FOUND FALSE)
  endif()
  return()
endif()
set(_enigma_machine_CONFIG_INCLUDED TRUE)

# output package information
if(NOT enigma_machine_FIND_QUIETLY)
  message(STATUS "Found enigma_machine: 0.0.0 (${enigma_machine_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'enigma_machine' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${enigma_machine_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(enigma_machine_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${enigma_machine_DIR}/${_extra}")
endforeach()
