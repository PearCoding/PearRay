# Locate OpenImageIO
# This module defines
#  OpenImageIO_FOUND, if false, do not try to link to OpenImageIO
#  OpenImageIO_LIBRARIES
#  OpenImageIO_INCLUDE_DIR
#
# Setups two targets
#  OpenImageIO::Core
#  OpenImageIO::Util
#
# Calling convection: <OpenImageIO/version.h>

SET(OpenImageIO_FOUND FALSE)
SET(OpenImageIO_INCLUDE_DIRS)
SET(OpenImageIO_LIBRARIES)

set(_def_search_paths
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  "${CMAKE_LIBRARY_PATH}"
  "${OIIO_DIR}"
  "${OPENIMAGEIO_DIR}"
  "${OpenImageIO_DIR}"
)

# Old version.h is deprecated!
find_path(OpenImageIO_INCLUDE_DIR OpenImageIO/oiioversion.h
  HINTS
    ENV OPENIMAGEIO_HOME OIIO_HOME
  PATH_SUFFIXES include/ local/include/
  PATHS ${_def_search_paths}
)

get_filename_component(_OpenImageIO_ROOT_DIR ${OpenImageIO_INCLUDE_DIR} DIRECTORY)

# Extract the version
if(OpenImageIO_INCLUDE_DIR)
  set(OpenImageIO_INCLUDE_DIRS "${OpenImageIO_INCLUDE_DIR}")

  file(READ "${OpenImageIO_INCLUDE_DIRS}/OpenImageIO/oiioversion.h" _version_file)
  string(REGEX REPLACE ".*#define OIIO_VERSION_MAJOR ([0-9]+).*" "\\1"
      OpenImageIO_VERSION_MAJOR "${_version_file}")
  string(REGEX REPLACE ".*#define OIIO_VERSION_MINOR ([0-9]+).*" "\\1"
      OpenImageIO_VERSION_MINOR "${_version_file}")
  string(REGEX REPLACE ".*#define OIIO_VERSION_PATCH ([0-9]+).*" "\\1"
      OpenImageIO_VERSION_PATCH "${_version_file}")
  set(OpenImageIO_VERSION "${OpenImageIO_VERSION_MAJOR}.${OpenImageIO_VERSION_MINOR}.${OpenImageIO_VERSION_PATCH}")
endif()

function(_OpenImageIO_FIND_LIB setname names)
  find_library(${setname}
    NAMES ${names}
    HINTS
      ENV OPENIMAGEIO_HOME OIIO_HOME
    PATH_SUFFIXES lib
    PATHS ${_def_search_paths} "${_OpenImageIO_ROOT_DIR}"
  )
endfunction()

# Allowed components
set(_OpenImageIO_COMPONENTS
  Core
  Util
)

set(_OpenImageIO_Core_DEPS )
set(_OpenImageIO_Core_LIBNAMES OpenImageIO)
set(_OpenImageIO_Util_DEPS Core)
set(_OpenImageIO_Util_LIBNAMES OpenImageIO_Util)


# If COMPONENTS ARE NOT GIVEN, set it to default
if(NOT OpenImageIO_FIND_COMPONENTS)
  set(OpenImageIO_FIND_COMPONENTS ${_OpenImageIO_COMPONENTS})
endif()

foreach(component ${OpenImageIO_FIND_COMPONENTS})
  if(NOT ${component} IN_LIST _OpenImageIO_COMPONENTS)
    message(ERROR "Unknown component ${component}")
  endif()

  foreach(_n ${_OpenImageIO_${component}_LIBNAMES})
    set(_release_names ${_release_names} "${_n}")
    set(_debug_names ${_debug_names} "${_n}d" "${_n}_d")
  endforeach(_n)

  _OpenImageIO_FIND_LIB(OpenImageIO_${component}_LIBRARY_RELEASE ${_release_names})
  if(OpenImageIO_${component}_LIBRARY_RELEASE)
    _OpenImageIO_FIND_LIB(OpenImageIO_${component}_LIBRARY_DEBUG ${_debug_names})
    if(NOT OpenImageIO_${component}_LIBRARY_DEBUG)
      set(OpenImageIO_${component}_LIBRARY_DEBUG "${OpenImageIO_${component}_LIBRARY_RELEASE}")
    endif()

    if(NOT "${OpenImageIO_${component}_LIBRARY_RELEASE}" STREQUAL "${OpenImageIO_${component}_LIBRARY_DEBUG}")
      set(OpenImageIO_${component}_LIBRARY
        optimized "${OpenImageIO_${component}_LIBRARY_RELEASE}" debug "${OpenImageIO_${component}_LIBRARY_DEBUG}")
    else()
      set(OpenImageIO_${component}_LIBRARY "${OpenImageIO_${component}_LIBRARY_RELEASE}")
    endif()

    if(OpenImageIO_${component}_LIBRARY AND EXISTS "${OpenImageIO_${component}_LIBRARY}")
      set(OpenImageIO_${component}_FOUND TRUE)
    endif()

    set(OpenImageIO_LIBRARIES ${OpenImageIO_LIBRARIES} ${OpenImageIO_${component}_LIBRARY})
  endif()

  mark_as_advanced(OpenImageIO_${component}_LIBRARY_RELEASE OpenImageIO_${component}_LIBRARY_DEBUG)
endforeach(component)

# Setup targets
foreach(component ${OpenImageIO_FIND_COMPONENTS})
  if(OpenImageIO_${component}_FOUND)
    set(_lib_release "${OpenImageIO_${component}_LIBRARY_RELEASE}")
    set(_lib_debug "${OpenImageIO_${component}_LIBRARY_DEBUG}")

    foreach(dependency ${_OpenImageIO_${component}_DEPS})
      set(_lib_release ${_lib_release} "${OpenImageIO_${dependency}_LIBRARY_RELEASE}")
      set(_lib_debug ${_lib_debug} "${OpenImageIO_${dependency}_LIBRARY_DEBUG}")
    endforeach(dependency)

    add_library(OpenImageIO::${component} SHARED IMPORTED)
    set_target_properties(OpenImageIO::${component} PROPERTIES
      ## TODO: Set this to a proper value
      IMPORTED_LOCATION_RELEASE ${_lib_release}
      IMPORTED_LOCATION_DEBUG ${_lib_debug}
      IMPORTED_IMPLIB_RELEASE ${_lib_release}
      IMPORTED_IMPLIB_DEBUG ${_lib_debug}
      INTERFACE_INCLUDE_DIRECTORIES ${OpenImageIO_INCLUDE_DIRS}
    )
  endif()
endforeach(component)

if (OpenImageIO_INCLUDE_DIRS AND OpenImageIO_LIBRARIES)
	set(OpenImageIO_FOUND TRUE)
endif()

if(OpenImageIO_FOUND AND NOT OpenImageIO_FIND_QUIETLY)
  message(STATUS "OpenImageIO version: ${OpenImageIO_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenImageIO
  VERSION_VAR OpenImageIO_VERSION
  REQUIRED_VARS OpenImageIO_LIBRARIES OpenImageIO_INCLUDE_DIRS
  HANDLE_COMPONENTS)

mark_as_advanced(OpenImageIO_INCLUDE_DIR)