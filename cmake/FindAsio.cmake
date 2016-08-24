# Locate Asio
# This module defines
#  ASIO_FOUND, if false, try not to use the Asio
#  ASIO_INCLUDE_DIR
#
# This is a header only library!
# Calling convection: <asio.hpp>

SET(ASIO_FOUND FALSE)
SET(ASIO_INCLUDE_DIRS)

SET(_Paths ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
  /usr
  ${CMAKE_LIBRARY_PATH}
  ${ASIO_DIR})
 
find_path(ASIO_INCLUDE_DIR asio.hpp
  HINTS
    ENV ASIO_HOME
  PATH_SUFFIXES include local/include 
  PATHS ${_Paths}
)

IF(ASIO_INCLUDE_DIR)
	SET(ASIO_INCLUDE_DIRS ${ASIO_INCLUDE_DIR})
ENDIF()

IF (ASIO_INCLUDE_DIR)
	SET(ASIO_FOUND TRUE)
ENDIF()

IF (ASIO_FOUND)
	IF (NOT ASIO_FIND_QUIETLY)
		MESSAGE(STATUS "Found Asio: ${ASIO_INCLUDE_DIR}")
	ENDIF (NOT ASIO_FIND_QUIETLY)
ELSE ()
	IF (ASIO_FIND_QUIETLY)
		MESSAGE(FATAL_ERROR "Could not find Asio")
	ENDIF (ASIO_FIND_QUIETLY)
ENDIF ()

mark_as_advanced(ASIO_INCLUDE_DIR)

