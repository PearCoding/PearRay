CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(PearRay)

#SETS
SET(VENDOR "PearRay project 2015-2020")
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 11)
SET(VERSION_HEX 0x0011)
SET(PR_Version ${VERSION_MAJOR}.${VERSION_MINOR})

if(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)
   SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
else()
	SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
SET(CMAKE_DEBUG_POSTFIX  "_d")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

#CHECKS
MESSAGE(STATUS "Building PearRay ${PR_Version}")

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
option(PR_WITH_CNT_VIEWER "Compile the additional PearRay CNT Viewer based on the QT library. Only useful for development." OFF)
option(PR_WITH_RDMP_VIEWER "Compile the additional PearRay Ray Dump Viewer based on the QT library. Only useful for development." OFF)
option(PR_BUILD_TESTS "Build tests." ON)
option(PR_BUILD_TESTS_PYTHON "Build tests for the python API. Requires PR_BUILD_TESTS to be ON" ON)
option(PR_GENERATE_COVERAGE "Generate coverage for debug builds - Currently only supported with the GCC compiler." ON)
option(PR_BUILD_DOCUMENTATION "Build documentation with doxygen." ON)
option(PR_CPP_CHECK "Do style check with cpplint." OFF)
option(PR_USE_SSE3 "Use SSE3 simd support." ON)
option(PR_USE_SSSE3 "Use SSSE3 simd support." ON)
option(PR_USE_SSE4 "Use SSE4_1 simd support." ON)
option(PR_USE_AVX "Use AVX simd support." OFF)
option(PR_USE_AVX2 "Use AVX2 simd support." OFF)
option(PR_USE_AVX512 "Use AVX512F simd support." OFF)

option(VERBOSE "Show debug information." OFF)
option(OIIO_LINK_STATIC "Link the static OpenImageIO libraries" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Boost_USE_MULTITHREADED ON)
#set(BOOST_ALL_DYN_LINK ON)

find_package(Boost 1.45 COMPONENTS filesystem system iostreams program_options regex REQUIRED)

IF(PR_WITH_CNT_VIEWER OR PR_WITH_RDMP_VIEWER)
	find_package(Qt5Widgets)
ENDIF()
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(OpenImageIO REQUIRED)
find_package(TBB REQUIRED)

IF(OIIO_LINK_STATIC)# If static, the dependencies of OIIO has to be included aswell.
	find_package(PNG REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(TIFF REQUIRED)
	find_package(OpenEXR REQUIRED)
	SET(OIIO_LIBRARIES ${PNG_LIBRARIES} ${JPEG_LIBRARIES}
	 ${TIFF_LIBRARIES} ${OPENEXR_LIBRARIES} ${OIIO_LIBRARIES})
ENDIF()

IF(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
	include(Coveralls)
	coveralls_turn_on_coverage()
ENDIF()

#DEFINITIONS AND FLAGS
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${Boost_LIBRARY_DIRS})

IF(PR_WITH_PYTHON)
	add_subdirectory(external/pybind11)
	SET(PR_HAS_PYTHON_API ON)
ENDIF()

# Due to problems with OpenEXR and C++17 we still use C++14 (Aug 2018)
# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(NOT PR_WITH_ASSERTS)
	SET(DEFS ${DEFS} DL_NO_ASSERTS PM_NO_ASSERTS PR_NO_ASSERTS)
ENDIF()

#CONFIGURE
SET(DL_TARGET "datalisp")
add_subdirectory(external/DataLisp)

SET(DEFS_DEBUG "${DEFS_DEBUG};PR_DEBUG")

IF(WIN32)
	SET(DEFS ${DEFS} OPENEXR_DLL)
ENDIF()

IF(OIIO_LINK_STATIC)
	SET(DEFS "${DEFS};OIIO_STATIC_BUILD")
ENDIF()

IF(PR_WITH_VIEWER)
	IF(${Qt5Widgets_FOUND})
		SET(DEFS "${DEFS};PR_WITH_VIEWER")
	ENDIF()
ENDIF()

configure_file(src/library/PR_Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/PR_Config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${EIGEN3_INCLUDE_DIR}
			external/DataLisp/src ${CMAKE_CURRENT_BINARY_DIR}/external/DataLisp
			external/tinyobjloader external/libsimdpp
			${OIIO_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${TBB_INCLUDE_DIRS}
			src/library src/utils)

# Setup CPPLINT
IF(PR_CPP_CHECK)
	SET(PR_CPPLINT_CMD "cpplint" CACHE FILEPATH "cpplint python script")

	set(PR_CPPLINT_STYLE)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/braces,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/tab,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/line_length,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/comments,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/indent,)
	#set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include_order,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/namespaces,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include_what_you_use,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-legal/copyright,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-readability/namespace,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-readability/todo,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-runtime/references,)

	function(add_cpplint_target TARGET_NAME SOURCES_LIST)
	add_custom_target(${TARGET_NAME}
		COMMAND "${CMAKE_COMMAND}" -E chdir
				"${CMAKE_CURRENT_SOURCE_DIR}"
				"${PR_CPPLINT_CMD}"
				"--filter=${PR_CPPLINT_STYLE}"
				"--counting=detailed"
				"--extensions=cpp,h,inl"
				"--headers=h,inl"
				"--linelength=80"
				${SOURCES_LIST}
		DEPENDS ${SOURCES_LIST}
		COMMENT "Checking C++ Style ${TARGET_NAME}"
		VERBATIM)
	add_dependencies(cppcheck ${TARGET_NAME})
	endfunction()

	add_custom_target(cppcheck)
ENDIF()

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
	add_subdirectory(src/viewer/utils)
	IF(PR_WITH_CNT_VIEWER)
		add_subdirectory(src/viewer/cnt)
	ENDIF()
	IF(PR_WITH_RDMP_VIEWER)
		add_subdirectory(src/viewer/rdmp)
	ENDIF()
ENDIF()

# Tests
IF(PR_BUILD_TESTS)
	add_subdirectory(src/sandbox)
	enable_testing()
	add_subdirectory(src/tests)
ENDIF()

# DOCUMENTATION
IF(PR_BUILD_DOCUMENTATION)
  find_package(Doxygen)
  IF(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tools/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    add_custom_target(doc_pearray
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
  ENDIF()
ENDIF()

# Coverage
IF(PR_GENERATE_COVERAGE AND (CMAKE_BUILD_TYPE MATCHES Debug))
file(GLOB_RECURSE HDR_FILES_CLIENT "src/client/*.h")
file(GLOB_RECURSE HDR_FILES_LIBRARY "src/library/*.h")
file(GLOB_RECURSE HDR_FILES_PYTHON "src/python/*.h")
file(GLOB_RECURSE HDR_FILES_UTILS "src/utils/*.h")

file(GLOB_RECURSE HDR_IN_FILES_CLIENT "src/client/*.inl")
file(GLOB_RECURSE HDR_IN_FILES_LIBRARY "src/library/*.inl")
file(GLOB_RECURSE HDR_IN_FILES_PYTHON "src/python/*.inl")
file(GLOB_RECURSE HDR_IN_FILES_UTILS "src/utils/*.inl")

file(GLOB_RECURSE SRC_FILES_CLIENT "src/client/*.cpp")
file(GLOB_RECURSE SRC_FILES_LIBRARY "src/library/*.cpp")
file(GLOB_RECURSE SRC_FILES_PYTHON "src/python/*.cpp")
file(GLOB_RECURSE SRC_FILES_UTILS "src/utils/*.cpp")

SET(CV_FILES ${HDR_FILES_CLIENT} ${HDR_FILES_LIBRARY} ${HDR_FILES_OSL} ${HDR_FILES_PYTHON} ${HDR_FILES_UTILS}
		${HDR_IN_FILES_CLIENT} ${HDR_IN_FILES_LIBRARY} ${HDR_IN_FILES_OSL} ${HDR_IN_FILES_PYTHON} ${HDR_IN_FILES_UTILS}
		${SRC_FILES_CLIENT} ${SRC_FILES_LIBRARY} ${SRC_FILES_OSL} ${SRC_FILES_PYTHON} ${SRC_FILES_UTILS})

coveralls_setup("${CV_FILES}" ON)
ENDIF()
