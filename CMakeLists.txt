CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

PROJECT(PearRay)

#SETS
SET(VENDOR "PearRay project 2015-2017")
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 10)
SET(VERSION_HEX 0x0010)
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
option(PR_ONLY_LIBRARY "Compile only the main and util library." OFF)
option(PR_ONLY_LIBRARY_SUBS "Compile only the main libraries in the subprojects." ON)
option(PR_WITH_ASSERTS "Compile with asserts on. Disable only in deployment code!" ON)
option(PR_WITH_GPU_SUPPORT "Build framework with GPU support if available." ON)
option(PR_WITH_PYTHON "Compile with python API." ON)
option(PR_WITH_OSL "Compile with OSL support if available." ON)
#option(PR_WITH_NETWORK "Enable network support." ON)
option(PR_USE_TRIANGLE_WATERTIGHT "Use Watertight triangle intersection algorithm instead of Trombore." OFF)
option(PR_BUILD_VIEWER "Compile the additional PearRay Viewer based on the QT library." ON)
option(PR_BUILD_TESTS "Build tests." ON)
option(PR_BUILD_PROFILE "Build with internal profiling system. Only recommended for development." OFF)
option(PR_CPP_CHECK "Do style check with cpplint." OFF)
option(PR_ENABLE_DIAGNOSIS "Enable diagnosis. Will slow down performance. Only for development." OFF)

option(VERBOSE "Show debug information." OFF)
option(OIIO_LINK_STATIC "Link the static OpenImageIO libraries" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Boost_USE_MULTITHREADED ON)
#set(BOOST_ALL_DYN_LINK ON)

find_package(Boost 1.45 COMPONENTS filesystem system iostreams program_options REQUIRED)
IF(PR_BUILD_VIEWER)
	find_package(Qt5Widgets)
ENDIF()
IF(PR_WITH_GPU_SUPPORT)
	find_package(OpenCL)
ENDIF()
find_package(OpenImageIO REQUIRED)
find_package(TBB REQUIRED)

IF(PR_WITH_OSL)
	find_package(OSL)
ENDIF()

IF(OIIO_LINK_STATIC)# If static, the dependencies of OIIO has to be included aswell.
	find_package(PNG REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(TIFF REQUIRED)
	find_package(OpenEXR REQUIRED)
	SET(OIIO_LIBRARIES ${PNG_LIBRARIES} ${JPEG_LIBRARIES}
	 ${TIFF_LIBRARIES} ${OPENEXR_LIBRARIES} ${OIIO_LIBRARIES})
ENDIF()

#DEFINITIONS AND FLAGS
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${Boost_LIBRARY_DIRS})

IF(PR_WITH_PYTHON)
	SET(TMP_BOOST_LIBS ${Boost_LIBRARIES})
	find_package(Boost 1.45 COMPONENTS python)
	IF(Boost_FOUND)
		find_package(PythonLibs)
		find_package(PythonInterp)
		IF(PythonLibs_FOUND AND PythonInterp_FOUND)
  			include_directories(${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
			find_package(NumPy)
			IF(NUMPY_FOUND)
  				include_directories(${NUMPY_INCLUDE_DIRS})
				SET(PR_HAS_PYTHON_API True)
			ENDIF()
		ENDIF()
	ENDIF()
	SET(Boost_LIBRARIES ${Boost_LIBRARIES} ${TMP_BOOST_LIBS})
ENDIF()

IF(MSVC)
	SET(DEFS "${DEFS};_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_WARNINGS;_SCL_SECURE_NO_WARNINGS;_CRT_SECURE_NO_DEPRECATE")
	#SET(DEFS_DEBUG "_DEBUG")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Oi /fp:fast /EHsc /W3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ob2 /Ot /GL")
	#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /MDd")
	#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
	IF(CMAKE_CL_64)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
	ENDIF()
	add_definitions( -DBOOST_ALL_NO_LIB )
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	IF (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
		if(VERBOSE)
        		message(STATUS "C++11 activated.")
		endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    ELSEIF (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
        message(WARNING "C++0x activated. If you get any errors update to a compiler which fully supports C++11")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
    ELSE ()
        message(WARNING "C++11 needed. Build can fail.")   
    ENDIF ()
ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	# We do not check version any further.
	if(VERBOSE)
		message(STATUS "C++11 activated.")
	endif()
	IF(WIN32)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd=c++11")
	ELSE()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	ENDIF()
ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# We do not check version any further.
	if(VERBOSE)
		message(STATUS "C++11 activated.")
	endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ELSE()
    message(WARNING "Unknown Compiler. C++11 needed. Build can fail.")   
ENDIF()

IF(NOT PR_WITH_ASSERTS)
	SET(DEFS ${DEFS} DL_NO_ASSERTS PM_NO_ASSERTS PR_NO_ASSERTS)
ENDIF()

#CONFIGURE
IF(PR_ONLY_LIBRARY_SUBS)
	SET(PM_ONLY_LIBRARY ON)
	SET(DL_ONLY_LIBRARY ON)
ENDIF(PR_ONLY_LIBRARY_SUBS)

SET(DL_TARGET "datalisp")

add_subdirectory(pearmath)
add_subdirectory(datalisp)

SET(DEFS "${DEFS};${PM_DEFS}")
SET(DEFS_DEBUG "${DEFS_DEBUG};${PM_DEFS_DEBUG};PR_DEBUG")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PM_COMPILE_FLAGS}")

IF(WIN32)
	SET(DEFS ${DEFS} OPENEXR_DLL)
ENDIF()

IF(OpenCL_FOUND)
	SET(PR_WITH_GPU)
	MESSAGE(STATUS "Using OpenCL")
ENDIF()

IF(OIIO_LINK_STATIC)
	SET(DEFS "${DEFS};OIIO_STATIC_BUILD")
ENDIF()

IF(${Qt5Widgets_FOUND} AND PR_BUILD_VIEWER)
	SET(DEFS "${DEFS};PR_WITH_VIEWER")
ENDIF()

IF(PR_BUILD_PROFILE)
	SET(DEFS "${DEFS};PR_PROFILE")
ENDIF()

IF(PR_ENABLE_DIAGNOSIS)
	SET(DEFS "${DEFS};PR_ENABLE_DIAGNOSIS")
ENDIF()

IF(PR_USE_TRIANGLE_WATERTIGHT)
	SET(TRIANGLE_TECHNIQUE 1)
ELSE()
	SET(TRIANGLE_TECHNIQUE 0)
ENDIF()

configure_file(src/library/PR_Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/PR_Config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} pearmath/src
			datalisp/src ${CMAKE_CURRENT_BINARY_DIR}/datalisp
			${OIIO_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${TBB_INCLUDE_DIRS}
			src/library)

IF(OpenCL_FOUND)
	include_directories(${OpenCL_INCLUDE_DIRS})
ENDIF()

# Setup CPPLINT
IF(PR_CPP_CHECK)
	if(NOT PythonInterp_FOUND)
		find_package(PythonInterp)
	endif()

	SET(PR_CPPLINT_CMD "cpplint" CACHE FILEPATH "cpplint python script")

	set(PR_CPPLINT_STYLE)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/braces,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/tab,)
	#set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include_order,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/namespaces,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include_what_you_use,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-legal/copyright,)
	set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-readability/namespace,)

	function(add_cpplint_target TARGET_NAME SOURCES_LIST)
	if(NOT PYTHONINTERP_FOUND)
		return()
	endif()

	add_custom_target(${TARGET_NAME}
		COMMAND "${CMAKE_COMMAND}" -E chdir
				"${CMAKE_CURRENT_SOURCE_DIR}"
				"${PYTHON_EXECUTABLE}"
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

	endfunction()
ENDIF()

# Libraries
add_subdirectory(src/library)
add_subdirectory(src/utils)

IF(PR_HAS_PYTHON_API)
	add_subdirectory(src/python)
ENDIF()

IF(OSL_FOUND)
	include_directories(${OSL_INCLUDE_DIR})
	add_subdirectory(src/osl)
ENDIF()

# Executables
add_subdirectory(src/client)

IF(${Qt5Widgets_FOUND} AND PR_BUILD_VIEWER)
	add_subdirectory(src/viewer)
ENDIF()

# Tests
IF(NOT PR_ONLY_LIBRARY AND PR_BUILD_TESTS)
	enable_testing()
	add_subdirectory(src/tests)
ENDIF()
