CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(PearRay)

#SETS
SET(VENDOR "PearRay project 2015")
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 4)
SET(VERSION_HEX 0x0004)
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
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets)

#DEFINITIONS AND FLAGS
link_directories(${CMAKE_CURRENT_BINARY_DIR})

IF(MSVC)
	#SET(DEFS "_UNICODE;UNICODE;_MT;_USRDLL")
	#SET(DEFS_DEBUG "_DEBUG")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Oi /fp:fast /EHsc")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ob2 /Ot /GL")
	#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /MDd")
	#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
	IF(CMAKE_CL_64)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
	ENDIF()
ELSE()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF()

SET(DEFS_DEBUG "${DEFS_DEBUG};PR_DEBUG")

#CONFIGURE
SET(PP_TARGET "pearpic")
SET(DL_TARGET "datalisp")

add_subdirectory(pearmath)
add_subdirectory(datalisp)
add_subdirectory(pearpic)

configure_file(${PR_Root}/library/Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} pearmath/src
			datalisp/src ${CMAKE_CURRENT_BINARY_DIR}/datalisp
			pearpic/src ${CMAKE_CURRENT_BINARY_DIR}/pearpic
			${PR_Root}/library ${PR_Root}/viewer ${PR_Root}/console ${PR_Root}/utils)

add_subdirectory(${PR_Root}/library)
add_subdirectory(${PR_Root}/utils)
add_subdirectory(${PR_Root}/console)

IF(${Qt5Widgets_FOUND})
	add_subdirectory(${PR_Root}/viewer)
ENDIF()