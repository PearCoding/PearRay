CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(PearRay)

#SETS
SET(VENDOR "PearRay project 2015-2016")
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 9)
SET(VERSION_HEX 0x0009)
SET(PR_Version ${VERSION_MAJOR}.${VERSION_MINOR})

IF(NOT DEFINED PR_TARGET)
	SET(PR_TARGET pr_lib)
ENDIF()

SET(PR_Root ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

if(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)
   SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
SET(CMAKE_DEBUG_POSTFIX  "_d")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

#CHECKS
MESSAGE(STATUS "Building PearRay ${PR_Version}")
MESSAGE(STATUS "Root: ${PR_Root}")

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
option(PR_NO_GPU "Build framework with only CPU support." OFF)
option(PR_BUILD_VIEWER "Compile the additional PearRay Viewer based on the QT library." ON)

option(OIIO_LINK_STATIC "Link the static OpenImageIO libraries" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets)
find_package(OpenCL)
find_package(OpenImageIO REQUIRED)
find_package(OSL REQUIRED)
find_package(Boost 1.36 COMPONENTS filesystem system iostreams program_options REQUIRED)

#DEFINITIONS AND FLAGS
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${Boost_LIBRARY_DIRS})

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
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	IF (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        message(STATUS "C++11 activated.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    ELSEIF (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
        message(WARNING "C++0x activated. If you get any errors update to a compiler which fully supports C++11")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
    ELSE ()
        message(WARNING "C++11 needed. Build can fail.")   
    ENDIF ()
ELSE()
    message(WARNING "Unknown Compiler. C++11 needed. Build can fail.")   
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

IF((NOT OpenCL_FOUND) OR PR_NO_GPU)
	SET(DEFS "${DEFS};PR_NO_GPU")
ELSE()
	MESSAGE(STATUS "Using OpenCL")
ENDIF()

IF(OIIO_LINK_STATIC)
	SET(DEFS "${DEFS};OIIO_STATIC_BUILD")
ENDIF()

IF(${Qt5Widgets_FOUND} AND PR_BUILD_VIEWER)
	SET(DEFS "${DEFS};PR_WITH_VIEWER")
ENDIF()

configure_file(${PR_Root}/library/Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} pearmath/src
			datalisp/src ${CMAKE_CURRENT_BINARY_DIR}/datalisp
			${OIIO_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${OSL_INCLUDE_DIR}
			${PR_Root}/library ${PR_Root}/viewer ${PR_Root}/console ${PR_Root}/utils ${PR_Root}/osl)

IF(OpenCL_FOUND AND NOT PR_NO_GPU)
include_directories(${OpenCL_INCLUDE_DIRS})
ENDIF()

add_subdirectory(${PR_Root}/library)
#add_subdirectory(${PR_Root}/osl)
add_subdirectory(${PR_Root}/utils)

IF(NOT PR_ONLY_LIBRARY)
	add_subdirectory(${PR_Root}/tests)
ENDIF()

IF(${Qt5Widgets_FOUND} AND PR_BUILD_VIEWER)
	add_subdirectory(${PR_Root}/viewer)
ENDIF()