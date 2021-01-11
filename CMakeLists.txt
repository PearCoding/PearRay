cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Prevent in source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	if(NOT DEFINED WITH_IN_SOURCE_BUILD)
		message(FATAL_ERROR "CMake generation for PearRay is not allowed within the source directory! Define WITH_IN_SOURCE_BUILD if absolutely necessary!" )
	endif()
endif()

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
	set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()

# Set default to Release
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Setup project
project(PearRay
	VERSION 0.14
	DESCRIPTION "Experimental high accurate spectral path and ray tracer")

set(PR_PLUGIN_VERSION "1.0")

# For whatever reason PearRay might be used as a subproject...
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(PR_SUBPROJECT OFF)
else()
	set(PR_SUBPROJECT ON)
endif()

# SETS
if(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)
   	set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
else()
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
endif()

set(INCLUDE_PREFIX "${PROJECT_NAME}")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_DEBUG_POSTFIX  "_d")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

set(PearRay_VENDOR "PearRay project 2015-2021")
include(Git)

#CHECKS
message(STATUS "Building PearRay ${PearRay_VERSION}")

if(MINGW)
	message(WARNING "Please don't use MinGW with PearRay, use the MSVC version instead")
endif()

if(MSYS)
	message(WARNING "Please don't use MSYS with PearRay, use the MSVC version instead")
endif()

if(CYGWIN)
	message(WARNING "Please don't use CYGWIN with PearRay, use the MSVC version instead")
endif()

if(NOT PR_SUBPROJECT)
    include(CTest)
endif()

#PACKAGES
include(CMakeDependentOption)

# Some defaults based on initial configuration
if(PR_SUBPROJECT)
	set(_default_extra OFF)
else()
	set(_default_extra ON)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(_default_lto ON)
else()
	set(_default_lto OFF)
endif()

if(MSVC)
	set(_default_ccache OFF)
else()
	set(_default_ccache ON)
endif()

option(PR_WITH_ASSERTS 			"Compile with asserts on. Disable only in deployment code!" ON)
option(PR_WITH_PYTHON 			"Compile with python API (using pybind11)" ON)
option(PR_WITH_CLIENT 			"Compile commandline client. Strongly recommended!" ON)
option(PR_WITH_MAIN_PLUGINS 	"Compile main plugins. Recommended!" ON)
option(PR_WITH_EXTRA_PLUGINS 	"Compile extra plugins which in general require extra dependencies" ON)
option(PR_EMBED_PLUGINS			"Embed plugins into utility library" ON)
option(PR_WITH_PROFILER 		"Compile with internal profiler. Not recommended in deployment code" OFF)
option(PR_WITH_EXTRA_TOOLS		"Compile supplementary tools" ON)
option(PR_GENERATE_COVERAGE 	"Generate coverage for debug builds - Currently only supported with the GCC compiler" OFF)

option(PR_BUILD_TESTS 			"Build tests" ${_default_extra})
option(PR_BUILD_DOCUMENTATION 	"Build documentation with doxygen" ${_default_extra})
option(PR_USE_LTO 				"Use linked time optimization if available" ${_default_lto})
option(PR_USE_CCACHE 			"Use ccache if available" ${_default_ccache})

cmake_dependent_option(PR_BUILD_TESTS_PYTHON "Build tests for the python API" ON "PR_BUILD_TESTS" OFF)

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

set(_qt_components Core Widgets Charts)
if(WIN32)
	set(_qt_components ${_qt_components} WinExtras)
endif()
find_package(Qt5 COMPONENTS ${_qt_components})

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(OpenImageIO REQUIRED)
find_package(Embree REQUIRED)
find_package(TBB REQUIRED COMPONENTS tbb)

find_package(ZLIB REQUIRED)

# OpenImageIO refers to OpenEXR header files!
# This is eminent in windows, but actually also a problem on linux. Need a handling in OpenImageIO package handler
if(WIN32)
  find_package(OpenEXR REQUIRED)
endif()

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
	add_subdirectory(external/pybind11 EXCLUDE_FROM_ALL)
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
add_subdirectory(external/DataLisp EXCLUDE_FROM_ALL)

# Detect hardware
add_subdirectory(src/hardware)

configure_file(src/base/PR_Config.h.in              ${CMAKE_CURRENT_BINARY_DIR}/PR_Config.h)
configure_file(src/base/config/HW_Switch.inl.in     ${CMAKE_CURRENT_BINARY_DIR}/config/HW_Switch.inl)
configure_file(src/base/config/Git.h.in             ${CMAKE_CURRENT_BINARY_DIR}/config/Git.h)
configure_file(src/base/config/Version.h.in         ${CMAKE_CURRENT_BINARY_DIR}/config/Version.h)

# Install generated header files
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PR_Config.h 
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${INCLUDE_PREFIX}" 
	COMPONENT development)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config/HW_Switch.inl
			  ${CMAKE_CURRENT_BINARY_DIR}/config/Git.h
			  ${CMAKE_CURRENT_BINARY_DIR}/config/Version.h
		 DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${INCLUDE_PREFIX}/config"
		 COMPONENT development)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
			external/DataLisp/src ${CMAKE_CURRENT_BINARY_DIR}/external/DataLisp
			external/tinyobjloader external/cxxopts/include)

# Extras
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(Strip)
include(LTO)
include(ExtraUtils)
include(Deploy)
include(Install)

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
add_subdirectory(src/skysun)
add_subdirectory(src/vcm)

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
	message(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")
	add_subdirectory(src/ui)
endif()

if(PR_WITH_EXTRA_TOOLS)
	add_subdirectory(src/tools)
endif()

# Tests
if(PR_BUILD_TESTS AND BUILD_TESTING)
	add_subdirectory(src/sandbox)
	add_subdirectory(src/tests)
endif()

# Documentation
if(PR_BUILD_DOCUMENTATION)
  include(Documentation)
endif()

# Coverage
if(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
	coveralls_setup()
endif()
