# Locate OpenSubdiv
# This module defines
#  OpenSubdiv_FOUND, if false, do not try to link to OpenSubdiv
#  OpenSubdiv_LIBRARIES
#  OpenSubdiv_INCLUDE_DIR
#
# Setups two targets
#  OpenSubdiv::Core
#  OpenSubdiv::Util
#
# Calling convection: <opensubdiv/version.h>

if(OpenSubdiv_FOUND)
  return()
endif()

SET(OpenSubdiv_FOUND FALSE)
SET(OpenSubdiv_INCLUDE_DIRS)
SET(OpenSubdiv_LIBRARIES)

set(_def_search_paths
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  "${CMAKE_LIBRARY_PATH}"
  "${OPENSUBDIV_DIR}"
  "${OpenSubdiv_DIR}"
)

set(_version_file opensubdiv/version.h)
find_path(OpenSubdiv_INCLUDE_DIR ${_version_file}
  HINTS
    ENV OPENSUBDIV_HOME
  PATH_SUFFIXES include/ local/include/
  PATHS ${_def_search_paths}
)

get_filename_component(_OpenSubdiv_ROOT_DIR ${OpenSubdiv_INCLUDE_DIR} DIRECTORY)

# Extract the version
if(OpenSubdiv_INCLUDE_DIR)
  set(OpenSubdiv_INCLUDE_DIRS "${OpenSubdiv_INCLUDE_DIR}")

  file(READ "${OpenSubdiv_INCLUDE_DIRS}/${_version_file}" _version_file)
  string(REGEX REPLACE ".*#define OPENSUBDIV_VERSION_MAJOR ([0-9]+).*" "\\1"
      OpenSubdiv_VERSION_MAJOR "${_version_file}")
  string(REGEX REPLACE ".*#define OPENSUBDIV_VERSION_MINOR ([0-9]+).*" "\\1"
      OpenSubdiv_VERSION_MINOR "${_version_file}")
  string(REGEX REPLACE ".*#define OPENSUBDIV_VERSION_PATCH ([0-9]+).*" "\\1"
      OpenSubdiv_VERSION_PATCH "${_version_file}")
  set(OpenSubdiv_VERSION "${OpenSubdiv_VERSION_MAJOR}.${OpenSubdiv_VERSION_MINOR}.${OpenSubdiv_VERSION_PATCH}")
endif()

function(_OpenSubdiv_FIND_LIB setname)
  find_library(${setname}
    NAMES ${ARGN}
    HINTS
      ENV OPENSUBDIV_HOME
    PATH_SUFFIXES lib
    PATHS ${_def_search_paths} "${_OpenSubdiv_ROOT_DIR}"
  )
endfunction()

# Allowed components
set(_OpenSubdiv_COMPONENTS
  CPU
  GPU
)

set(_OpenSubdiv_CPU_DEPS )
set(_OpenSubdiv_CPU_LIBNAMES osdCPU)
set(_OpenSubdiv_GPU_DEPS )
set(_OpenSubdiv_GPU_LIBNAMES osdGPU)


# If COMPONENTS are not given, set it to default
if(NOT OpenSubdiv_FIND_COMPONENTS)
  set(OpenSubdiv_FIND_COMPONENTS ${_OpenSubdiv_COMPONENTS})
endif()

foreach(component ${OpenSubdiv_FIND_COMPONENTS})
  if(NOT ${component} IN_LIST _OpenSubdiv_COMPONENTS)
    message(ERROR "Unknown component ${component}")
  endif()

  set(_release_names )
  set(_debug_names )
  foreach(_n ${_OpenSubdiv_${component}_LIBNAMES})
    set(_release_names ${_release_names} "${_n}")
    set(_debug_names ${_debug_names} "${_n}d" "${_n}_d")
  endforeach(_n)

  _OpenSubdiv_FIND_LIB(OpenSubdiv_${component}_LIBRARY_RELEASE ${_release_names})
  if(OpenSubdiv_${component}_LIBRARY_RELEASE)
    _OpenSubdiv_FIND_LIB(OpenSubdiv_${component}_LIBRARY_DEBUG ${_debug_names} ${_release_names})
    if(NOT OpenSubdiv_${component}_LIBRARY_DEBUG)
      set(OpenSubdiv_${component}_LIBRARY_DEBUG "${OpenSubdiv_${component}_LIBRARY_RELEASE}")
    endif()

    if(NOT "${OpenSubdiv_${component}_LIBRARY_RELEASE}" STREQUAL "${OpenSubdiv_${component}_LIBRARY_DEBUG}")
      set(OpenSubdiv_${component}_LIBRARY
        optimized "${OpenSubdiv_${component}_LIBRARY_RELEASE}" debug "${OpenSubdiv_${component}_LIBRARY_DEBUG}")
    else()
      set(OpenSubdiv_${component}_LIBRARY "${OpenSubdiv_${component}_LIBRARY_RELEASE}")
    endif()

    if(OpenSubdiv_${component}_LIBRARY AND EXISTS "${OpenSubdiv_${component}_LIBRARY}")
      set(OpenSubdiv_${component}_FOUND TRUE)
    endif()

    set(OpenSubdiv_LIBRARIES ${OpenSubdiv_LIBRARIES} ${OpenSubdiv_${component}_LIBRARY})
  endif()

  mark_as_advanced(OpenSubdiv_${component}_LIBRARY_RELEASE OpenSubdiv_${component}_LIBRARY_DEBUG)
endforeach(component)

# Setup targets
foreach(component ${OpenSubdiv_FIND_COMPONENTS})
  if(OpenSubdiv_${component}_FOUND)
    set(_deps )

    foreach(dependency ${_OpenSubdiv_${component}_DEPS})
      set(_deps ${_deps} OpenSubdiv::${component})
    endforeach(dependency)

    add_library(OpenSubdiv::${component} SHARED IMPORTED)
    set_target_properties(OpenSubdiv::${component} PROPERTIES
      IMPORTED_LOCATION_RELEASE         "${OpenSubdiv_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_MINSIZEREL      "${OpenSubdiv_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_RELWITHDEBINFO  "${OpenSubdiv_${component}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_DEBUG           "${OpenSubdiv_${component}_LIBRARY_DEBUG}"
      ## TODO: Set this to a proper value (on windows -> dll)
      IMPORTED_IMPLIB_RELEASE           "${OpenSubdiv_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_MINSIZEREL        "${OpenSubdiv_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_RELWITHDEBINFO    "${OpenSubdiv_${component}_LIBRARY_RELEASE}"
      IMPORTED_IMPLIB_DEBUG             "${OpenSubdiv_${component}_LIBRARY_DEBUG}"
      INTERFACE_LINK_LIBRARIES          "${_deps}"
      INTERFACE_INCLUDE_DIRECTORIES     "${OpenSubdiv_INCLUDE_DIRS}"
    )
  endif()
endforeach(component)

set(_OSD_INC_DIR "${OpenSubdiv_INCLUDE_DIR}/opensubdiv/osd")
set(OpenSubdiv_COMPUTE_FEATURES )
if(OpenSubdiv_CPU_FOUND)
list(APPEND OpenSubdiv_COMPUTE_FEATURES "CPU")
if(EXISTS "${_OSD_INC_DIR}/ompEvaluator.h")
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    set(OpenSubdiv_HAS_OPENMP ON)
    set_property(TARGET OpenSubdiv::CPU APPEND PROPERTY INTERFACE_LINK_LIBRARIES OpenMP::OpenMP_CXX)
    set_property(TARGET OpenSubdiv::CPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_OPENMP")
    list(APPEND OpenSubdiv_COMPUTE_FEATURES "OMP")
  endif()
endif()
if(EXISTS "${_OSD_INC_DIR}/tbbEvaluator.h")
  find_package(TBB)
  if(TBB_FOUND)
    set(OpenSubdiv_HAS_TBB ON)
    set_property(TARGET OpenSubdiv::CPU APPEND PROPERTY INTERFACE_LINK_LIBRARIES TBB::tbb)
    set_property(TARGET OpenSubdiv::CPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_TBB")
    list(APPEND OpenSubdiv_COMPUTE_FEATURES "TBB")
  endif()
endif()
endif()

if(OpenSubdiv_GPU_FOUND)
if(EXISTS "${_OSD_INC_DIR}/clEvaluator.h")
  set(OpenSubdiv_HAS_OPENCL ON)
  set_property(TARGET OpenSubdiv::GPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_OPENCL")
  list(APPEND OpenSubdiv_COMPUTE_FEATURES "OPENCL")
endif()
if(EXISTS "${_OSD_INC_DIR}/cudaEvaluator.h")
  set(OpenSubdiv_HAS_CUDA ON)
  set_property(TARGET OpenSubdiv::GPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_CUDA")
  list(APPEND OpenSubdiv_COMPUTE_FEATURES "CUDA")
endif()
if(EXISTS "${_OSD_INC_DIR}/d3d11ComputeEvaluator.h")
  set(OpenSubdiv_HAS_DXCOMPUTE ON)
  set_property(TARGET OpenSubdiv::GPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_DXCOMPUTE")
  list(APPEND OpenSubdiv_COMPUTE_FEATURES "DXCOMPUTE")
endif()
if(EXISTS "${_OSD_INC_DIR}/glComputeEvaluator.h")
  set(OpenSubdiv_HAS_GLCOMPUTE ON)
  set_property(TARGET OpenSubdiv::GPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_GLCOMPUTE")
  list(APPEND OpenSubdiv_COMPUTE_FEATURES "GLCOMPUTE")
endif()
if(EXISTS "${_OSD_INC_DIR}/mtlComputeEvaluator.h")
  set(OpenSubdiv_HAS_MTLCOMPUTE ON)
  set_property(TARGET OpenSubdiv::GPU APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "OPENSUBDIV_HAS_MTLCOMPUTE")
  list(APPEND OpenSubdiv_COMPUTE_FEATURES "MTLCOMPUTE")
endif()
endif()

if (OpenSubdiv_INCLUDE_DIRS AND OpenSubdiv_LIBRARIES)
	set(OpenSubdiv_FOUND TRUE)
endif()

if(OpenSubdiv_FOUND AND NOT OpenSubdiv_FIND_QUIETLY)
  message(STATUS "OpenSubdiv version: ${OpenSubdiv_VERSION}, computation: ${OpenSubdiv_COMPUTE_FEATURES}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSubdiv
  VERSION_VAR OpenSubdiv_VERSION
  REQUIRED_VARS OpenSubdiv_LIBRARIES OpenSubdiv_INCLUDE_DIRS
  HANDLE_COMPONENTS)

mark_as_advanced(OpenSubdiv_INCLUDE_DIR)