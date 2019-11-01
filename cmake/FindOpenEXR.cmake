# Locate OpenEXR
# This module defines
#  OpenEXR_FOUND, if false, do not try to link to OpenEXR
#  OpenEXR_LIBRARIES
#  OpenEXR_INCLUDE_DIR
#
# Setups two targets
#  OpenEXR::IlmImf
#  OpenEXR::IlmImfUtil
#
# Calling convection: <OpenEXR/OpenEXRConfig.h>

SET(OpenEXR_FOUND FALSE)
SET(OpenEXR_INCLUDE_DIRS)
SET(OpenEXR_LIBRARIES)

set(_def_search_paths
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  "${CMAKE_LIBRARY_PATH}"
  "${OPENEXR_DIR}"
  "${OpenEXR_DIR}"
)

find_path(OpenEXR_INCLUDE_DIR OpenEXR/OpenEXRConfig.h
  HINTS
    ENV OPENEXR_HOME
  PATH_SUFFIXES include/ local/include/
  PATHS ${_def_search_paths}
)

get_filename_component(_OpenEXR_ROOT_DIR ${OpenEXR_INCLUDE_DIR} DIRECTORY)

# Extract the version
if(OpenEXR_INCLUDE_DIR)
  set(OpenEXR_INCLUDE_DIRS "${OpenEXR_INCLUDE_DIR}")

  file(READ "${OpenEXR_INCLUDE_DIRS}/OpenEXR/OpenEXRConfig.h" _openexr_file)
  string(REGEX REPLACE ".*#define OPENEXR_VERSION_MAJOR ([0-9]+).*" "\\1"
      OpenEXR_VERSION_MAJOR "${_openexr_file}")
  string(REGEX REPLACE ".*#define OPENEXR_VERSION_MINOR ([0-9]+).*" "\\1"
      OpenEXR_VERSION_MINOR "${_openexr_file}")
  string(REGEX REPLACE ".*#define OPENEXR_VERSION_PATCH ([0-9]+).*" "\\1"
      OpenEXR_VERSION_PATCH "${_openexr_file}")
  set(OpenEXR_VERSION "${OpenEXR_VERSION_MAJOR}.${OpenEXR_VERSION_MINOR}.${OpenEXR_VERSION_PATCH}")
endif()

function(_OpenEXR_FIND_LIB setname)
  find_library(${setname}
    NAMES ${ARGN}
    HINTS
      ENV OPENEXR_HOME
    PATH_SUFFIXES lib
    PATHS ${_def_search_paths} "${_OpenEXR_ROOT_DIR}"
  )
endfunction()

# Allowed components
set(_OpenEXR_COMPONENTS
  IlmImf
  IlmImfUtil
)

set(_OpenEXR_IlmImf_DEPS )
set(_OpenEXR_IlmImfUtil_DEPS IlmImf)

# If COMPONENTS ARE NOT GIVEN, set it to default
if(NOT OpenEXR_FIND_COMPONENTS)
  set(OpenEXR_FIND_COMPONENTS ${_OpenEXR_COMPONENTS})
endif()

foreach(component ${OpenEXR_FIND_COMPONENTS})
  if(NOT ${component} IN_LIST _OpenEXR_COMPONENTS)
    message(ERROR "Unknown component ${component}")
  endif()

  _OpenEXR_FIND_LIB(OpenEXR_${component}_LIBRARY_RELEASE ${component})
  if(OpenEXR_${component}_LIBRARY_RELEASE)
    _OpenEXR_FIND_LIB(OpenEXR_${component}_LIBRARY_DEBUG ${component}d ${component}_d ${component})
    if(NOT OpenEXR_${component}_LIBRARY_DEBUG)
      set(OpenEXR_${component}_LIBRARY_DEBUG "${OpenEXR_${component}_LIBRARY_RELEASE}")
    endif()

    if(NOT "${OpenEXR_${component}_LIBRARY_RELEASE}" STREQUAL "${OpenEXR_${component}_LIBRARY_DEBUG}")
      set(OpenEXR_${component}_LIBRARY
        optimized "${OpenEXR_${component}_LIBRARY_RELEASE}" debug "${OpenEXR_${component}_LIBRARY_DEBUG}")
    else()
      set(OpenEXR_${component}_LIBRARY "${OpenEXR_${component}_LIBRARY_RELEASE}")
    endif()

    if(OpenEXR_${component}_LIBRARY AND EXISTS "${OpenEXR_${component}_LIBRARY}")
      set(OpenEXR_${component}_FOUND TRUE)
    endif()

    set(OpenEXR_LIBRARIES ${OpenEXR_LIBRARIES} ${OpenEXR_${component}_LIBRARY})
  endif()

  mark_as_advanced(OpenEXR_${component}_LIBRARY_RELEASE OpenEXR_${component}_LIBRARY_DEBUG)
endforeach(component)

# Setup targets
foreach(component ${OpenEXR_FIND_COMPONENTS})
  if(OpenEXR_${component}_FOUND)
  set(_deps )

  foreach(dependency ${OpenEXR_${component}_DEPS})
    set(_deps ${_deps} OpenEXR::${component})
  endforeach(dependency)

    add_library(OpenEXR::${component} SHARED IMPORTED)
    set_target_properties(OpenEXR::${component} PROPERTIES
      ## TODO: Set this to a proper value (on windows -> dll)
      IMPORTED_LOCATION_RELEASE         "${OpenEXR_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_MINSIZEREL      "${OpenEXR_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_RELWITHDEBINFO  "${OpenEXR_${component}_LIBRARY_DEBUG}"
      IMPORTED_LOCATION_DEBUG           "${OpenEXR_${component}_LIBRARY_DEBUG}"
      IMPORTED_IMPLIB_RELEASE           "${OpenEXR_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_MINSIZEREL        "${OpenEXR_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_RELWITHDEBINFO    "${OpenEXR_${component}_LIBRARY_DEBUG}"
      IMPORTED_IMPLIB_DEBUG             "${OpenEXR_${component}_LIBRARY_DEBUG}"
      INTERFACE_LINK_LIBRARIES          "${_deps}"
      INTERFACE_INCLUDE_DIRECTORIES     "${OpenEXR_INCLUDE_DIRS}"
    )
  endif()
endforeach(component)

if (OpenEXR_INCLUDE_DIRS AND OpenEXR_LIBRARIES)
	set(OpenEXR_FOUND TRUE)
endif()

if(OpenEXR_FOUND AND NOT OpenEXR_FIND_QUIETLY)
  message(STATUS "OpenEXR version: ${OpenEXR_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR
  VERSION_VAR OpenEXR_VERSION
  REQUIRED_VARS OpenEXR_LIBRARIES OpenEXR_INCLUDE_DIRS
  HANDLE_COMPONENTS)

mark_as_advanced(OpenEXR_INCLUDE_DIR)