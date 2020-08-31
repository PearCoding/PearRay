cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(PearRay
	VERSION 0.13
	DESCRIPTION "Experimental high accurate spectral path and ray tracer")

set(PR_PLUGIN_VERSION "1.0")

#SETS
if(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)
   	set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
else()
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_DEBUG_POSTFIX  "_d")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

set(VENDOR "PearRay project 2015-2020")
include(Git)

#CHECKS
MESSAGE(STATUS "Building PearRay ${PearRay_VERSION}")

if(MINGW)
	MESSAGE(WARNING "Please don't use MinGW with PearRay, use the MSVC version instead")
endif()

if(MSYS)
	MESSAGE(WARNING "Please don't use MSYS with PearRay, use the MSVC version instead")
endif()

if(CYGWIN)
	MESSAGE(WARNING "Please don't use CYGWIN with PearRay, use the MSVC version instead")
endif()

#PACKAGES
include(CMakeDependentOption)

option(PR_WITH_ASSERTS 			"Compile with asserts on. Disable only in deployment code!" ON)
option(PR_WITH_PYTHON 			"Compile with python API (using pybind11)." ON)
option(PR_WITH_CLIENT 			"Compile commandline client. Strongly recommended!" ON)
option(PR_WITH_MAIN_PLUGINS 	"Compile main plugins. Recommended!" ON)
option(PR_WITH_EXTRA_PLUGINS 	"Compile extra plugins which in general require extra dependencies." ON)
option(PR_EMBED_PLUGINS			"Embed plugins into utility library" ON)
option(PR_WITH_PROFILER 		"Compile with internal profiler." ON)
option(PR_WITH_EXTRA_TOOLS		"Compile supplementary tools." ON)
option(PR_BUILD_TESTS 			"Build tests." ON)
option(PR_GENERATE_COVERAGE 	"Generate coverage for debug builds - Currently only supported with the GCC compiler." OFF)
option(PR_BUILD_DOCUMENTATION 	"Build documentation with doxygen." ON)

CMAKE_DEPENDENT_OPTION(PR_BUILD_TESTS_PYTHON 	"Build tests for the python API. Requires PR_BUILD_TESTS to be ON" ON
	"NOT PR_BUILD_TESTS" OFF)
CMAKE_DEPENDENT_OPTION(PR_USE_LTO 				"Use linked time optimization if available." ON
	"NOT CMAKE_BUILD_TYPE STREQUAL \"Release\"" OFF)
CMAKE_DEPENDENT_OPTION(PR_USE_CCACHE 			"Use ccache if available." ON
	"MSVC" OFF)

# march should be default, as we use simdpp
option(PR_OPTIMIZE_FOR_NATIVE	"Build with -march=native if possible" ON)

option(PR_EXTRA_OPENSUBDIV		"Build extra OpenSubDiv plugin" ON)
option(PR_EXTRA_SEEXPR			"Build extra SeExpr plugin" ON)
option(PR_EXTRA_RGL_BRDF 		"Download BRDF Loader by RGL-EPFL and build rgl-measured material plugin" ON)
option(PR_EXTRA_DAYLIGHT 		"Build plugins useful for daylight simulation" ON)

# Hardware feature switches
option(PR_DISABLE_HW_FEATURE_SSE3 		"Disable SSE3 support" OFF)
option(PR_DISABLE_HW_FEATURE_SSSE3 		"Disable SSSE3 support" OFF)
option(PR_DISABLE_HW_FEATURE_SSE4_1 	"Disable SSE4.1 support" OFF)
option(PR_DISABLE_HW_FEATURE_SSE4_2 	"Disable SSE4.2 support" OFF)
option(PR_DISABLE_HW_FEATURE_AVX		"Disable AVX support" OFF)
option(PR_DISABLE_HW_FEATURE_AVX2 		"Disable AVX2 support" OFF)
option(PR_DISABLE_HW_FEATURE_AVX512F 	"Disable AVX512F support" OFF)
option(PR_DISABLE_HW_FEATURE_AVX512BW 	"Disable AVX512BW support" OFF)
option(PR_DISABLE_HW_FEATURE_AVX512DQ 	"Disable AVX512DQ support" OFF)
option(PR_DISABLE_HW_FEATURE_AVX512VL 	"Disable AVX512VL support" OFF)
option(PR_DISABLE_HW_FEATURE_POPCNT 	"Disable POPCNT support" OFF)
option(PR_DISABLE_HW_FEATURE_FMA4 		"Disable FMA4 support" OFF)
option(PR_DISABLE_HW_FEATURE_FMA 		"Disable FMA3 support" OFF)

option(BUILD_SHARED_LIBS		"Build shared library" OFF)

# Include packages
if(NOT WIN32)
	find_package(Threads REQUIRED)
endif()

find_package(Filesystem REQUIRED)

if(PR_WITH_DIAGNOSTIC)
	find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
endif()

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(OpenImageIO REQUIRED)
find_package(Embree REQUIRED)
find_package(TBB REQUIRED COMPONENTS tbb)

# Optional
if(PR_USE_CCACHE)
	include(CCache)
endif()

if(PR_EXTRA_OPENSUBDIV)
	find_package(OpenSubdiv COMPONENTS CPU)
endif()

if(PR_EXTRA_SEEXPR)
	find_package(SeExpr2)
endif()

#DEFINITIONS AND FLAGS
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${Boost_LIBRARY_DIRS})

# Python API
if(PR_WITH_PYTHON)
	add_subdirectory(external/pybind11)
	set(PR_HAS_PYTHON_API ON)
endif()

if(NOT PR_WITH_ASSERTS)
	add_definitions(-DDL_NO_ASSERTS -DPR_NO_ASSERTS)
endif()

# Configure
set(DL_WITH_TOOLS OFF CACHE BOOL "")
set(DL_WITH_PYTHON OFF CACHE BOOL "")
set(DL_BUILD_TESTS OFF CACHE BOOL "")
set(DL_BUILD_DOCUMENTATION OFF CACHE BOOL "")
add_subdirectory(external/DataLisp)

# Detect hardware
add_subdirectory(src/hardware)

configure_file(src/base/PR_Config.h.in              ${CMAKE_CURRENT_BINARY_DIR}/PR_Config.h)
configure_file(src/base/config/HW_Switch.inl.in     ${CMAKE_CURRENT_BINARY_DIR}/config/HW_Switch.inl)
configure_file(src/base/config/Git.h.in             ${CMAKE_CURRENT_BINARY_DIR}/config/Git.h)
configure_file(src/base/config/Version.h.in         ${CMAKE_CURRENT_BINARY_DIR}/config/Version.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
			external/DataLisp/src ${CMAKE_CURRENT_BINARY_DIR}/external/DataLisp
			external/tinyobjloader external/cxxopts/include
			${OIIO_INCLUDE_DIR})

# Extras
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(Strip)
include(LTO)
include(ExtraUtils)
include(Deploy)

if(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
	include(Coveralls)
	coveralls_turn_on_coverage()
endif()

# march
include(CheckCXXCompilerFlag)
if(PR_OPTIMIZE_FOR_NATIVE)
	CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
	if(COMPILER_SUPPORTS_MARCH_NATIVE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	endif()
endif()

if(WIN32)
	if(PR_USE_AVX512)
		CHECK_CXX_COMPILER_FLAG("/arch:AVX512" COMPILER_SUPPORTS_AVX512)
		if(COMPILER_SUPPORTS_AVX512)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX512")
		endif()
	elseif(PR_USE_AVX2)
		CHECK_CXX_COMPILER_FLAG("/arch:AVX2" COMPILER_SUPPORTS_AVX2)
		if(COMPILER_SUPPORTS_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
		endif()
	elseif(PR_USE_AVX)
		CHECK_CXX_COMPILER_FLAG("/arch:AVX" COMPILER_SUPPORTS_AVX)
		if(COMPILER_SUPPORTS_AVX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
		endif()
	endif()
endif()

# Libraries
add_subdirectory(src/base)
add_subdirectory(src/core)

# If embedded, plugins have to be imported first, as they are added TO pr_lib_utils
if(PR_EMBED_PLUGINS)
	add_subdirectory(src/plugins)
endif()

add_subdirectory(src/loader)

# If not embedded, plugins have to be imported later, as they are depended ON pr_lib_utils
if(NOT PR_EMBED_PLUGINS)
	add_subdirectory(src/plugins)
endif()

if(PR_HAS_PYTHON_API)
	add_subdirectory(src/python)
endif()

# Executables
if(PR_WITH_CLIENT)
	add_subdirectory(src/client)
endif()

if(${Qt5Widgets_FOUND})
	add_subdirectory(src/ui)
endif()

if(PR_WITH_EXTRA_TOOLS)
	add_subdirectory(src/tools)
endif()

# Tests
include(CTest) 
if(PR_BUILD_TESTS)
	add_subdirectory(src/sandbox)
	if(BUILD_TESTING)
		add_subdirectory(src/tests)
	endif()
endif()

# Documentation
if(PR_BUILD_DOCUMENTATION)
  include(Documentation)
endif()

# Coverage
if(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
	include(Coverage)
endif()
