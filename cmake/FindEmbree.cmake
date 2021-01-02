# Locate Intel Embree 3
# This module defines
#  EMBREE_FOUND, if false, do not try to link to Embree
#  EMBREE_LIBRARIES
#  EMBREE_INCLUDE_DIR
#
# Calling convection: <embree3/rtcore.h>

if(EMBREE_FOUND)
  return()
endif()

SET(EMBREE_FOUND FALSE)
SET(EMBREE_INCLUDE_DIRS)
SET(EMBREE_LIBRARIES)

set(_def_search_paths
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  "${CMAKE_LIBRARY_PATH}"
  "${EMBREE_DIR}"
  "${EMBREE_DIR}"
)

find_path(EMBREE_INCLUDE_DIR embree3/rtcore.h
  HINTS
    ENV EMBREE_HOME EMBREE_HOME
  PATH_SUFFIXES include/ local/include/
  PATHS ${_def_search_paths}
)

get_filename_component(_EMBREE_ROOT_DIR ${EMBREE_INCLUDE_DIR} DIRECTORY)

# Extract the version
if(EMBREE_INCLUDE_DIR)
  set(EMBREE_INCLUDE_DIRS "${EMBREE_INCLUDE_DIR}")

  file(READ "${EMBREE_INCLUDE_DIRS}/embree3/rtcore_config.h" _version_file)
  string(REGEX REPLACE ".*#define RTC_VERSION_MAJOR ([0-9]+).*" "\\1"
      EMBREE_VERSION_MAJOR "${_version_file}")
  string(REGEX REPLACE ".*#define RTC_VERSION_MINOR ([0-9]+).*" "\\1"
      EMBREE_VERSION_MINOR "${_version_file}")
  string(REGEX REPLACE ".*#define RTC_VERSION_PATCH ([0-9]+).*" "\\1"
      EMBREE_VERSION_PATCH "${_version_file}")
  set(EMBREE_VERSION "${EMBREE_VERSION_MAJOR}.${EMBREE_VERSION_MINOR}.${EMBREE_VERSION_PATCH}")
endif()

function(_EMBREE_FIND_LIB setname)
  find_library(${setname}
    NAMES ${ARGN}
    HINTS
      ENV EMBREE_HOME EMBREE_HOME
    PATH_SUFFIXES lib
    PATHS ${_def_search_paths} "${_EMBREE_ROOT_DIR}"
  )
endfunction()

# Allowed components
set(_EMBREE_COMPONENTS
  Embree
)

set(_EMBREE_Embree_DEPS )
set(_EMBREE_Embree_LIBNAMES embree3)

# If COMPONENTS are not given, set it to default
if(NOT EMBREE_FIND_COMPONENTS)
  set(EMBREE_FIND_COMPONENTS ${_EMBREE_COMPONENTS})
endif()

foreach(component ${EMBREE_FIND_COMPONENTS})
  if(NOT ${component} IN_LIST _EMBREE_COMPONENTS)
    message(ERROR "Unknown component ${component}")
  endif()

  set(_release_names )
  set(_debug_names )
  foreach(_n ${_EMBREE_${component}_LIBNAMES})
    set(_release_names ${_release_names} "${_n}")
    set(_debug_names ${_debug_names} "${_n}d" "${_n}_d")
  endforeach(_n)

  _EMBREE_FIND_LIB(EMBREE_${component}_LIBRARY_RELEASE ${_release_names})
  if(EMBREE_${component}_LIBRARY_RELEASE)
    _EMBREE_FIND_LIB(EMBREE_${component}_LIBRARY_DEBUG ${_debug_names} ${_release_names})
    if(NOT EMBREE_${component}_LIBRARY_DEBUG)
      set(EMBREE_${component}_LIBRARY_DEBUG "${EMBREE_${component}_LIBRARY_RELEASE}")
    endif()

    if(NOT "${EMBREE_${component}_LIBRARY_RELEASE}" STREQUAL "${EMBREE_${component}_LIBRARY_DEBUG}")
      set(EMBREE_${component}_LIBRARY
        optimized "${EMBREE_${component}_LIBRARY_RELEASE}" debug "${EMBREE_${component}_LIBRARY_DEBUG}")
    else()
      set(EMBREE_${component}_LIBRARY "${EMBREE_${component}_LIBRARY_RELEASE}")
    endif()

    if(EMBREE_${component}_LIBRARY AND EXISTS "${EMBREE_${component}_LIBRARY}")
      set(EMBREE_${component}_FOUND TRUE)
    endif()

    set(EMBREE_LIBRARIES ${EMBREE_LIBRARIES} ${EMBREE_${component}_LIBRARY})
  endif()

  mark_as_advanced(EMBREE_${component}_LIBRARY_RELEASE EMBREE_${component}_LIBRARY_DEBUG)
endforeach(component)

# Setup targets
foreach(component ${EMBREE_FIND_COMPONENTS})
  if(EMBREE_${component}_FOUND)
    set(_deps )

    foreach(dependency ${_EMBREE_${component}_DEPS})
      set(_deps ${_deps} Embree::${component})
    endforeach(dependency)

    add_library(Embree::${component} SHARED IMPORTED)
    set_target_properties(Embree::${component} PROPERTIES
      IMPORTED_LOCATION_RELEASE         "${EMBREE_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_MINSIZEREL      "${EMBREE_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_RELWITHDEBINFO  "${EMBREE_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_DEBUG           "${EMBREE_${component}_LIBRARY_DEBUG}"
      ## TODO: Set this to a proper value (on windows -> dll)
      IMPORTED_IMPLIB_RELEASE           "${EMBREE_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_MINSIZEREL        "${EMBREE_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_RELWITHDEBINFO    "${EMBREE_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_DEBUG             "${EMBREE_${component}_LIBRARY_DEBUG}"
      INTERFACE_LINK_LIBRARIES          "${_deps}"
      INTERFACE_INCLUDE_DIRECTORIES     "${EMBREE_INCLUDE_DIRS}"
    )
  endif()
endforeach(component)

if (EMBREE_INCLUDE_DIRS AND EMBREE_LIBRARIES)
	set(EMBREE_FOUND TRUE)
endif()

if(NOT EMBREE_FOUND)
  find_package(Embree QUIET NO_MODULE)

  if(TARGET embree)
    add_library(Embree::Embree SHARED IMPORTED)

    set_target_properties(Embree::Embree PROPERTIES
      INTERFACE_LINK_LIBRARIES embree
    )
  endif()
endif()

if(EMBREE_FOUND AND NOT EMBREE_FIND_QUIETLY)
  message(STATUS "Embree version: ${EMBREE_VERSION}")
endif()

set(Embree_FOUND ${EMBREE_FOUND})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Embree
  VERSION_VAR EMBREE_VERSION
  REQUIRED_VARS EMBREE_LIBRARIES EMBREE_INCLUDE_DIRS
  HANDLE_COMPONENTS)

mark_as_advanced(EMBREE_INCLUDE_DIR)