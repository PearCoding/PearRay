# Locate Intel TBB
# This module defines
#  TBB_FOUND, if false, do not try to link to OSL
#  TBB_LIBRARIES
#  TBB_INCLUDE_DIR
#
# Calling convection: <tbb/tbb.h>

SET(TBB_FOUND FALSE)
SET(TBB_INCLUDE_DIRS)
SET(TBB_LIBRARIES)
SET(TBB_LIBRARIES_RELEASE)
SET(TBB_LIBRARIES_DEBUG)

SET(SEARCH_PATHS 
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/intel/tbb
  /opt/local
  /opt/csw
  /opt
  /usr
  ${CMAKE_LIBRARY_PATH}
  ${TBB_DIR})

function(TBB_FIND_LIB setname names)
find_library(${setname}
  NAMES ${names}
  HINTS
  ENV TBB_HOME
  PATH_SUFFIXES lib
  PATHS ${SEARCH_PATHS}
)
endfunction()

macro(TBB_FIND_COMPONENT component)
  TBB_FIND_LIB(TBB_${component}_LIBRARY_RELEASE ${component})
  IF(TBB_${component}_LIBRARY_RELEASE)
    list(APPEND TBB_LIBRARIES_RELEASE "${TBB_${component}_LIBRARY_RELEASE}")
  ENDIF()
  TBB_FIND_LIB(TBB_${component}_LIBRARY_DEBUG ${component}_debug)
  IF(TBB_${component}_LIBRARY_DEBUG)
    list(APPEND TBB_LIBRARIES_DEBUG "${TBB_${component}_LIBRARY_DEBUG}")
  ENDIF()
  mark_as_advanced(TBB_${component}_LIBRARY_RELEASE TBB_${component}_LIBRARY_DEBUG)
endmacro()

find_path(TBB_INCLUDE_DIR tbb/tbb.h
  HINTS
  ENV TBB_HOME
  PATH_SUFFIXES include local/include 
  PATHS ${SEARCH_PATHS}
)
IF(TBB_INCLUDE_DIR)
	SET(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR})
ENDIF()

TBB_FIND_COMPONENT(tbb)
IF(";${TBB_FIND_COMPONENTS};" MATCHES ";tbbmalloc;")
  TBB_FIND_COMPONENT(tbbmalloc)
ENDIF()
IF(";${TBB_FIND_COMPONENTS};" MATCHES ";tbbmalloc_proxy;")
  TBB_FIND_COMPONENT(tbbmalloc_proxy)
ENDIF()
IF(";${TBB_FIND_COMPONENTS};" MATCHES ";tbb_preview;")
  TBB_FIND_COMPONENT(tbb_preview)
ENDIF()

IF(NOT TBB_LIBRARIES_DEBUG)
    SET(TBB_LIBRARIES ${TBB_LIBRARIES_RELEASE})
ELSEIF(NOT TBB_LIBRARIES_RELEASE)
    SET(TBB_LIBRARIES ${TBB_LIBRARIES_DEBUG})
ELSE()
    foreach(LIB ${TBB_LIBRARIES_RELEASE})
      list(APPEND TBB_LIBRARIES optimized ${LIB})
    endforeach()
    foreach(LIB ${TBB_LIBRARIES_DEBUG})
      list(APPEND TBB_LIBRARIES debug ${LIB})
    endforeach()
ENDIF()

IF (TBB_INCLUDE_DIR AND TBB_LIBRARIES)
	SET(TBB_FOUND TRUE)
ENDIF()

IF (TBB_FOUND)
	IF (NOT TBB_FIND_QUIETLY)
		MESSAGE(STATUS "Found TBB: ${TBB_INCLUDE_DIR} LIBS: ${TBB_LIBRARIES}")
	ENDIF (NOT TBB_FIND_QUIETLY)
ELSE (TBB_FOUND)
	IF (TBB_FIND_QUIETLY)
		MESSAGE(FATAL_ERROR "Could not find TBB")
	ENDIF (TBB_FIND_QUIETLY)
ENDIF (TBB_FOUND)

mark_as_advanced(TBB_INCLUDE_DIR)
mark_as_advanced(TBB_LIBRARIES_DEBUG)
mark_as_advanced(TBB_LIBRARIES_RELEASE)

