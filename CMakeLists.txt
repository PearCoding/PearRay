cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(PearRay
	VERSION 0.12
	DESCRIPTION "Experimental raytracer")

#SETS
if(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)
   	SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
else()
	SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
SET(CMAKE_DEBUG_POSTFIX  "_d")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

SET(VENDOR "PearRay project 2015-2020")
include(Git)

#CHECKS
MESSAGE(STATUS "Building PearRay ${PearRay_VERSION}")

IF(MINGW)
	MESSAGE(WARNING "Please don't use MinGW with PearRay, use the MSVC version instead")
ENDIF()

IF(MSYS)
	MESSAGE(WARNING "Please don't use MSYS with PearRay, use the MSVC version instead")
ENDIF()

IF(CYGWIN)
	MESSAGE(WARNING "Please don't use CYGWIN with PearRay, use the MSVC version instead")
ENDIF()

#PACKAGES
option(PR_WITH_ASSERTS "Compile with asserts on. Disable only in deployment code!" ON)
option(PR_WITH_PYTHON "Compile with python API (using pybind11)." ON)
option(PR_WITH_UTILITY_LIBRARY "Compile utility library. Strongly recommended!" ON)
option(PR_WITH_CLIENT "Compile commandline client. Strongly recommended!" ON)
option(PR_WITH_MAIN_PLUGINS "Compile main plugins. Recommended!" ON)
option(PR_WITH_EXTRA_PLUGINS "Compile extra plugins." ON)
option(PR_WITH_DIAGNOSTIC "Compile the additional PearRayDiagnostic Viewer based on the QT library. Useful if no exr viewer is available" ON)
option(PR_BUILD_TESTS "Build tests." ON)
option(PR_BUILD_TESTS_PYTHON "Build tests for the python API. Requires PR_BUILD_TESTS to be ON" ON)
option(PR_COMPRESS_SPEC_FILES "Compress spectral files with boost::zlib" ON)
option(PR_GENERATE_COVERAGE "Generate coverage for debug builds - Currently only supported with the GCC compiler." ON)
option(PR_BUILD_DOCUMENTATION "Build documentation with doxygen." ON)
option(PR_USE_SSE3 "Use SSE3 simd support." ON)
option(PR_USE_SSSE3 "Use SSSE3 simd support." ON)
option(PR_USE_SSE4 "Use SSE4_1 simd support." ON)
option(PR_USE_AVX "Use AVX simd support." OFF)
option(PR_USE_AVX2 "Use AVX2 simd support." OFF)
option(PR_USE_AVX512 "Use AVX512F simd support." OFF)
option(PR_USE_LTO "Use linked time optimization if available." ON)
option(PR_USE_LTO_ONLY_RELEASE "Use linked time optimization on release builds only." OFF)

option(VERBOSE "Show debug information." OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost 1.45 COMPONENTS filesystem system iostreams program_options regex REQUIRED)
if(WIN32)
	add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
endif()

IF(PR_WITH_DIAGNOSTIC)
	find_package(Qt5Widgets)
ENDIF()
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(OpenImageIO REQUIRED)
find_package(OpenEXR COMPONENTS IlmImf IlmImfUtil REQUIRED)
find_package(TBB REQUIRED)

IF(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
	include(Coveralls)
	coveralls_turn_on_coverage()
ENDIF()

#DEFINITIONS AND FLAGS
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${Boost_LIBRARY_DIRS})

# Python API
IF(PR_WITH_PYTHON)
	add_subdirectory(external/pybind11)
	set(PR_HAS_PYTHON_API ON)
ENDIF()

# Due to problems with OpenEXR and C++17 we still use C++14 (Aug 2018)
# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(NOT PR_WITH_ASSERTS)
	add_definitions(-DDL_NO_ASSERTS -DPR_NO_ASSERTS)
ENDIF()

# Configure
SET(DL_TARGET "datalisp")
add_subdirectory(external/DataLisp)

configure_file(src/library/PR_Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/PR_Config.h)
configure_file(src/library/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/Version.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
			external/DataLisp/src ${CMAKE_CURRENT_BINARY_DIR}/external/DataLisp
			external/tinyobjloader external/libsimdpp
			${OIIO_INCLUDE_DIR}
			src/library src/utils)

# Extras
include(Strip)
include(LTO)
include(ExtraUtils)
include(Deploy)

# Libraries
add_subdirectory(src/library)

IF(PR_WITH_UTILITY_LIBRARY)
	add_subdirectory(src/utils)
ENDIF(PR_WITH_UTILITY_LIBRARY)

add_subdirectory(src/plugins)

IF(PR_HAS_PYTHON_API)
	add_subdirectory(src/python)
ENDIF()

# Executables
IF(PR_WITH_CLIENT)
	add_subdirectory(src/client)
ENDIF(PR_WITH_CLIENT)

IF(${Qt5Widgets_FOUND})
	IF(PR_WITH_DIAGNOSTIC)
		add_subdirectory(src/diagnostic)
	ENDIF()
ENDIF()

# Tests
IF(PR_BUILD_TESTS)
	add_subdirectory(src/sandbox)
	enable_testing()
	add_subdirectory(src/tests)
ENDIF()

# DOCUMENTATION
if(PR_BUILD_DOCUMENTATION)
  include(Documentation)
endif()

# Coverage
if(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
	include(Coverage)
endif()
