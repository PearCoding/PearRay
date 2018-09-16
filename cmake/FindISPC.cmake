# Locate ISPC compiler
# This module defines
#  ISPC_FOUND
#  ISPC_EXECUTABLE
#  ISPC_VERSION

SET(ISPC_FOUND FALSE)
SET(ISPC_EXECUTABLE)
SET(ISPC_VERSION)

FIND_PROGRAM(ISPC_EXECUTABLE ispc DOC "Path to the ISPC executable.")

IF(ISPC_EXECUTABLE)
  SET(ISPC_FOUND TRUE)

  EXECUTE_PROCESS(COMMAND ${ISPC_EXECUTABLE} --version OUTPUT_VARIABLE ISPC_OUTPUT)
  STRING(REGEX MATCH " ([0-9]+[.][0-9]+[.][0-9]+)(dev|knl|ptx)? " DUMMY "${ISPC_OUTPUT}")
  SET(ISPC_VERSION ${CMAKE_MATCH_1})

	IF (NOT ISPC_FIND_QUIETLY)
    MESSAGE(STATUS "Found ISPC version ${ISPC_VERSION} (${ISPC_EXECUTABLE})")
  ENDIF ()

  SET(ISPC_VERSION ${ISPC_VERSION} CACHE STRING "ISPC Version")
  MARK_AS_ADVANCED(ISPC_VERSION)
  MARK_AS_ADVANCED(ISPC_EXECUTABLE)

  function(ISPC_COMPILE _obj_out)
    cmake_parse_arguments(_args "" "" "SOURCES" ${ARGN})

    SET (ISPC_TARGET_EXT ${CMAKE_CXX_OUTPUT_EXTENSION})
    SET (ISPC_ADDITIONAL_ARGS ${ISPC_ADDITIONAL_ARGS} --target=host --opt=force-aligned-memory)
    
    IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
      SET(ISPC_ARCHITECTURE "x86-64")
    ELSE()
      SET(ISPC_ARCHITECTURE "x86")
    ENDIF()
    
    SET(ISPC_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${ISPC_TARGET_DIR})
    
    IF(ISPC_INCLUDE_DIR)
      STRING(REPLACE ";" ";-I;" ISPC_INCLUDE_DIR_PARMS "${ISPC_INCLUDE_DIR}")
      SET(ISPC_INCLUDE_DIR_PARMS "-I" ${ISPC_INCLUDE_DIR_PARMS})
    ENDIF()
    
    SET(ISPC_OBJECTS )
    foreach (src ${_args_SOURCES})
      get_filename_component(fname ${src} NAME_WE)
      get_filename_component(dir   ${src} PATH)
      
      IF("${dir}" STREQUAL "")
        SET(outdir ${ISPC_TARGET_DIR})
      ELSE("${dir}" STREQUAL "")
        SET(outdir ${ISPC_TARGET_DIR}/${dir})
      ENDIF("${dir}" STREQUAL "")
      SET(outdirh ${ISPC_TARGET_DIR})

      SET(obj ${outdir}/${fname}.ispc${ISPC_TARGET_EXT})
      SET(hdr ${outdirh}/${fname}_ispc.h)
      SET(idep ${outdirh}/${fname}.ispc.idep)

      get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
      foreach(id ${inc_dirs})
        SET(ISPC_INCLUDE_DIR_PARMS ${ISPC_INCLUDE_DIR_PARMS} -I ${id})
      endforeach(id ${inc_dirs})
    
      SET(deps "")
      IF (EXISTS ${idep})
        FILE(READ ${idep} contents)
        STRING(REPLACE " " ";"     contents "${contents}")
        STRING(REPLACE ";" "\\\\;" contents "${contents}")
        STRING(REPLACE "\n" ";"    contents "${contents}")
        FOREACH(dep ${contents})
          IF (EXISTS ${dep})
            SET(deps ${deps} ${dep})
          ENDIF (EXISTS ${dep})
        ENDFOREACH(dep ${contents})
      ENDIF ()

      IF (WIN32)
        SET(ISPC_ADDITIONAL_ARGS ${ISPC_ADDITIONAL_ARGS} --dllexport)
      ELSE()
        SET(ISPC_ADDITIONAL_ARGS ${ISPC_ADDITIONAL_ARGS} --pic)
      ENDIF()

      ADD_CUSTOM_COMMAND(
        OUTPUT ${obj} ${hdr} ${idep}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir}
        COMMAND ${ISPC_EXECUTABLE}
        ${compile_flags}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${ISPC_INCLUDE_DIR_PARMS}
        --arch=${ISPC_ARCHITECTURE}
        ${ISPC_ADDITIONAL_ARGS}
        -h ${hdr}
        -MMM ${idep}
        -o ${obj}
        ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${deps}
        COMMENT "Building ISPC object ${obj}"
      )

      SET(ISPC_OBJECTS ${ISPC_OBJECTS} ${obj})
    endforeach()

    set(${_obj_out} ${ISPC_OBJECTS} PARENT_SCOPE)
  endfunction()
ELSE(ISPC_EXECUTABLE)
  IF (ISPC_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find ISPC")
	ENDIF (ISPC_FIND_REQUIRED)
endif(ISPC_EXECUTABLE)
