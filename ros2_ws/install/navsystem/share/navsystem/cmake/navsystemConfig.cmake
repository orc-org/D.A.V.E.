# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_navsystem_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED navsystem_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(navsystem_FOUND FALSE)
  elseif(NOT navsystem_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(navsystem_FOUND FALSE)
  endif()
  return()
endif()
set(_navsystem_CONFIG_INCLUDED TRUE)

# output package information
if(NOT navsystem_FIND_QUIETLY)
  message(STATUS "Found navsystem: 0.0.0 (${navsystem_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'navsystem' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${navsystem_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(navsystem_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${navsystem_DIR}/${_extra}")
endforeach()
