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

SET(Embree_FOUND FALSE)
SET(Embree_INCLUDE_DIRS)
SET(Embree_LIBRARIES)

set(_def_search_paths
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  "${CMAKE_LIBRARY_PATH}"
  "${EMBREE_DIR}"
  "${Embree_DIR}"
)

find_path(Embree_INCLUDE_DIR embree3/rtcore.h
  HINTS
    ENV Embree_HOME EMBREE_HOME
  PATH_SUFFIXES include/ local/include/
  PATHS ${_def_search_paths}
)

get_filename_component(_Embree_ROOT_DIR ${Embree_INCLUDE_DIR} DIRECTORY)

# Extract the version
if(Embree_INCLUDE_DIR)
  set(Embree_INCLUDE_DIRS "${Embree_INCLUDE_DIR}")

  file(READ "${Embree_INCLUDE_DIRS}/embree3/rtcore_config.h" _version_file)
  string(REGEX REPLACE ".*#define RTC_VERSION_MAJOR ([0-9]+).*" "\\1"
      Embree_VERSION_MAJOR "${_version_file}")
  string(REGEX REPLACE ".*#define RTC_VERSION_MINOR ([0-9]+).*" "\\1"
      Embree_VERSION_MINOR "${_version_file}")
  string(REGEX REPLACE ".*#define RTC_VERSION_PATCH ([0-9]+).*" "\\1"
      Embree_VERSION_PATCH "${_version_file}")
  set(Embree_VERSION "${Embree_VERSION_MAJOR}.${Embree_VERSION_MINOR}.${Embree_VERSION_PATCH}")
endif()

function(_Embree_FIND_LIB setname)
  find_library(${setname}
    NAMES ${ARGN}
    HINTS
      ENV Embree_HOME EMBREE_HOME
    PATH_SUFFIXES lib
    PATHS ${_def_search_paths} "${_Embree_ROOT_DIR}"
  )
endfunction()

# Allowed components
set(_Embree_COMPONENTS
  Embree
)

set(_Embree_Embree_DEPS )
set(_Embree_Embree_LIBNAMES embree3)

# If COMPONENTS are not given, set it to default
if(NOT Embree_FIND_COMPONENTS)
  set(Embree_FIND_COMPONENTS ${_Embree_COMPONENTS})
endif()

foreach(component ${Embree_FIND_COMPONENTS})
  if(NOT ${component} IN_LIST _Embree_COMPONENTS)
    message(ERROR "Unknown component ${component}")
  endif()

  set(_release_names )
  set(_debug_names )
  foreach(_n ${_Embree_${component}_LIBNAMES})
    set(_release_names ${_release_names} "${_n}")
    set(_debug_names ${_debug_names} "${_n}d" "${_n}_d")
  endforeach(_n)

  _Embree_FIND_LIB(Embree_${component}_LIBRARY_RELEASE ${_release_names})
  if(Embree_${component}_LIBRARY_RELEASE)
    _Embree_FIND_LIB(Embree_${component}_LIBRARY_DEBUG ${_debug_names} ${_release_names})
    if(NOT Embree_${component}_LIBRARY_DEBUG)
      set(Embree_${component}_LIBRARY_DEBUG "${Embree_${component}_LIBRARY_RELEASE}")
    endif()

    if(NOT "${Embree_${component}_LIBRARY_RELEASE}" STREQUAL "${Embree_${component}_LIBRARY_DEBUG}")
      set(Embree_${component}_LIBRARY
        optimized "${Embree_${component}_LIBRARY_RELEASE}" debug "${Embree_${component}_LIBRARY_DEBUG}")
    else()
      set(Embree_${component}_LIBRARY "${Embree_${component}_LIBRARY_RELEASE}")
    endif()

    if(Embree_${component}_LIBRARY AND EXISTS "${Embree_${component}_LIBRARY}")
      set(Embree_${component}_FOUND TRUE)
    endif()

    set(Embree_LIBRARIES ${Embree_LIBRARIES} ${Embree_${component}_LIBRARY})
  endif()

  mark_as_advanced(Embree_${component}_LIBRARY_RELEASE Embree_${component}_LIBRARY_DEBUG)
endforeach(component)

# Setup targets
foreach(component ${Embree_FIND_COMPONENTS})
  if(Embree_${component}_FOUND)
    set(_deps )

    foreach(dependency ${_Embree_${component}_DEPS})
      set(_deps ${_deps} Embree::${component})
    endforeach(dependency)

    add_library(Embree::${component} SHARED IMPORTED)
    set_target_properties(Embree::${component} PROPERTIES
      IMPORTED_LOCATION_RELEASE         "${Embree_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_MINSIZEREL      "${Embree_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_RELWITHDEBINFO  "${Embree_${component}_LIBRARY_DEBUG}"
      IMPORTED_LOCATION_DEBUG           "${Embree_${component}_LIBRARY_DEBUG}"
      ## TODO: Set this to a proper value (on windows -> dll)
      IMPORTED_IMPLIB_RELEASE           "${Embree_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_MINSIZEREL        "${Embree_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_RELWITHDEBINFO    "${Embree_${component}_LIBRARY_DEBUG}"
      IMPORTED_IMPLIB_DEBUG             "${Embree_${component}_LIBRARY_DEBUG}"
      INTERFACE_LINK_LIBRARIES          "${_deps}"
      INTERFACE_INCLUDE_DIRECTORIES     "${Embree_INCLUDE_DIRS}"
    )
  endif()
endforeach(component)

if (Embree_INCLUDE_DIRS AND Embree_LIBRARIES)
	set(Embree_FOUND TRUE)
endif()

if(Embree_FOUND AND NOT Embree_FIND_QUIETLY)
  message(STATUS "Embree version: ${Embree_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Embree
  VERSION_VAR Embree_VERSION
  REQUIRED_VARS Embree_LIBRARIES Embree_INCLUDE_DIRS
  HANDLE_COMPONENTS)

mark_as_advanced(Embree_INCLUDE_DIR)