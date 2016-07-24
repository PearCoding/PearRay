# Locate OpenShadingLanguage
# This module defines
#  OSL_FOUND, if false, do not try to link to OSL
#  OSL_LIBRARIES
#  OSL_INCLUDE_DIR
#
# Calling convection: <OSL/oslversion.h>

SET(OSL_FOUND FALSE)
SET(OSL_INCLUDE_DIRS)
SET(OSL_OSLCOMP_LIBRARY)
SET(OSL_OSLEXEC_LIBRARY)
SET(OSL_OSLQUERY_LIBRARY)

find_path(OSL_INCLUDE_DIR OSL/oslversion.h
  HINTS
    ENV OSL_DIR
  PATH_SUFFIXES include local/include 
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/csw # Blastwave
  /opt
  /usr
  ${CMAKE_LIBRARY_PATH}
)
IF(OSL_INCLUDE_DIR)
	SET(OSL_INCLUDE_DIRS ${OSL_INCLUDE_DIR})
ENDIF()

find_library(OSL_OSLCOMP_LIBRARY
  NAMES oslcomp
  HINTS
    ENV OSL_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
  /usr
  ${CMAKE_LIBRARY_PATH}
)

find_library(OSL_OSLEXEC_LIBRARY
  NAMES oslexec
  HINTS
    ENV OSL_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
  /usr
  ${CMAKE_LIBRARY_PATH}
)

find_library(OSL_OSLQUERY_LIBRARY
  NAMES oslquery
  HINTS
    ENV OSL_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
  /usr
  ${CMAKE_LIBRARY_PATH}
)

IF(OSL_OSLCOMP_LIBRARY AND OSL_OSLEXEC_LIBRARY AND OSL_OSLQUERY_LIBRARY)
	SET(OSL_LIBRARIES "${OSL_OSLCOMP_LIBRARY};${OSL_OSLEXEC_LIBRARY};${OSL_OSLQUERY_LIBRARY}")
ENDIF()

IF (OSL_INCLUDE_DIR AND OSL_OSLCOMP_LIBRARY AND OSL_OSLEXEC_LIBRARY AND OSL_OSLQUERY_LIBRARY)
	SET(OSL_FOUND TRUE)
ENDIF()

IF (OSL_FOUND)
	IF (NOT OSL_FIND_QUIETLY)
		MESSAGE(STATUS "Found OSL: ${OSL_INCLUDE_DIR} LIBS: ${OSL_LIBRARIES}")
	ENDIF (NOT OSL_FIND_QUIETLY)
ELSE (OSL_FOUND)
	IF (OSL_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find OSL")
	ENDIF (OSL_FIND_REQUIRED)
ENDIF (OSL_FOUND)

mark_as_advanced(OSL_INCLUDE_DIR OSL_OSLCOMP_LIBRARY OSL_OSLEXEC_LIBRARY OSL_OSLQUERY_LIBRARY)

