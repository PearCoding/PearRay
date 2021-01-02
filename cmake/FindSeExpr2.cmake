# Locate SeExpr2
# This module defines
#  SeExpr2_FOUND, if false, do not try to link to SeExpr2
#  SeExpr2_LIBRARIES
#  SeExpr2_INCLUDE_DIR
#
# Setups two targets
#  SeExpr2::Core
#  SeExpr2::Util
#
# Calling convection: <SeExpr2/ExprType.h>

if(SeExpr2_FOUND)
  return()
endif()

SET(SeExpr2_FOUND FALSE)
SET(SeExpr2_INCLUDE_DIRS)
SET(SeExpr2_LIBRARIES)

set(_def_search_paths
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  "${CMAKE_LIBRARY_PATH}"
  "${SEEXPR2_DIR}"
  "${SeExpr2_DIR}"
)

find_path(SeExpr2_INCLUDE_DIR SeExpr2/ExprType.h
  HINTS
    ENV SEEXPR2_HOME
  PATH_SUFFIXES include/ local/include/
  PATHS ${_def_search_paths}
)
set(SeExpr2_INCLUDE_DIRS ${SeExpr2_INCLUDE_DIR})

get_filename_component(_SeExpr2_ROOT_DIR ${SeExpr2_INCLUDE_DIR} DIRECTORY)

# Extract if LLVM is needed
if(SeExpr2_INCLUDE_DIR)
  file(READ "${SeExpr2_INCLUDE_DIR}/SeExpr2/ExprConfig.h" _config_file)
  string(REGEX MATCH "#if (1)" SeExpr2_HAS_LLVM "${_config_file}")
endif()

if(SeExpr2_HAS_LLVM)
  find_package(LLVM)
endif()

function(_SeExpr2_FIND_LIB setname)
  find_library(${setname}
    NAMES ${ARGN}
    HINTS
      ENV SEEXPR2_HOME
    PATH_SUFFIXES lib
    PATHS ${_def_search_paths} "${_SeExpr2_ROOT_DIR}"
  )
endfunction()

# Allowed components
set(_SeExpr2_COMPONENTS
  SeExpr2
)

set(_SeExpr2_SeExpr2_DEPS )
set(_SeExpr2_SeExpr2_LIBNAMES SeExpr2)


# If COMPONENTS are not given, set it to default
if(NOT SeExpr2_FIND_COMPONENTS)
  set(SeExpr2_FIND_COMPONENTS ${_SeExpr2_COMPONENTS})
endif()

foreach(component ${SeExpr2_FIND_COMPONENTS})
  if(NOT ${component} IN_LIST _SeExpr2_COMPONENTS)
    message(ERROR "Unknown component ${component}")
  endif()

  set(_release_names )
  set(_debug_names )
  foreach(_n ${_SeExpr2_${component}_LIBNAMES})
    set(_release_names ${_release_names} "${_n}")
    set(_debug_names ${_debug_names} "${_n}d" "${_n}_d")
  endforeach(_n)

  _SeExpr2_FIND_LIB(SeExpr2_${component}_LIBRARY_RELEASE ${_release_names})
  if(SeExpr2_${component}_LIBRARY_RELEASE)
    _SeExpr2_FIND_LIB(SeExpr2_${component}_LIBRARY_DEBUG ${_debug_names} ${_release_names})
    if(NOT SeExpr2_${component}_LIBRARY_DEBUG)
      set(SeExpr2_${component}_LIBRARY_DEBUG "${SeExpr2_${component}_LIBRARY_RELEASE}")
    endif()

    if(NOT "${SeExpr2_${component}_LIBRARY_RELEASE}" STREQUAL "${SeExpr2_${component}_LIBRARY_DEBUG}")
      set(SeExpr2_${component}_LIBRARY
        optimized "${SeExpr2_${component}_LIBRARY_RELEASE}" debug "${SeExpr2_${component}_LIBRARY_DEBUG}")
    else()
      set(SeExpr2_${component}_LIBRARY "${SeExpr2_${component}_LIBRARY_RELEASE}")
    endif()

    if(SeExpr2_${component}_LIBRARY AND EXISTS "${SeExpr2_${component}_LIBRARY}")
      set(SeExpr2_${component}_FOUND TRUE)
    endif()

    set(SeExpr2_LIBRARIES ${SeExpr2_LIBRARIES} ${SeExpr2_${component}_LIBRARY})
  endif()

  mark_as_advanced(SeExpr2_${component}_LIBRARY_RELEASE SeExpr2_${component}_LIBRARY_DEBUG)
endforeach(component)

# Setup targets
foreach(component ${SeExpr2_FIND_COMPONENTS})
  if(SeExpr2_${component}_FOUND)
    set(_deps )

    foreach(dependency ${_SeExpr2_${component}_DEPS})
      set(_deps ${_deps} SeExpr2::${component})
    endforeach(dependency)

    add_library(SeExpr2::${component} SHARED IMPORTED)
    set_target_properties(SeExpr2::${component} PROPERTIES
      IMPORTED_LOCATION_RELEASE         "${SeExpr2_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_MINSIZEREL      "${SeExpr2_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_RELWITHDEBINFO  "${SeExpr2_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_DEBUG           "${SeExpr2_${component}_LIBRARY_DEBUG}"
      ## TODO: Set this to a proper value (on windows -> dll)
      IMPORTED_IMPLIB_RELEASE           "${SeExpr2_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_MINSIZEREL        "${SeExpr2_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_RELWITHDEBINFO    "${SeExpr2_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_DEBUG             "${SeExpr2_${component}_LIBRARY_DEBUG}"
      INTERFACE_LINK_LIBRARIES          "${_deps}"
      INTERFACE_INCLUDE_DIRECTORIES     "${SeExpr2_INCLUDE_DIRS};${LLVM_INCLUDE_DIR}"
    )
  endif()
endforeach(component)

if (SeExpr2_INCLUDE_DIRS AND SeExpr2_LIBRARIES)
	set(SeExpr2_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SeExpr2
  REQUIRED_VARS SeExpr2_LIBRARIES SeExpr2_INCLUDE_DIRS
  HANDLE_COMPONENTS)

mark_as_advanced(SeExpr2_INCLUDE_DIR)