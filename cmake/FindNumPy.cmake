# Locate NumPy C api
# This module defines
#  NUMPY_FOUND
#  NUMPY_INCLUDE_DIRS
#  NUMPY_VERSION
#  NUMPY_VERSION_MAJOR
#  NUMPY_VERSION_MINOR
#
# Calling convection: <numpy/arrayobject.h>
# (Based on https://github.com/ndarray/Boost.NumPy/blob/master/libs/numpy/cmake/FindNumPy.cmake from Boost.NumPy)
#
# This version expects PYTHON_EXECUTABLE to be available

execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
    "import numpy as n; print(n.__version__); print(n.get_include());"
    RESULT_VARIABLE _NUMPY_SEARCH_SUCCESS
    OUTPUT_VARIABLE _NUMPY_VALUES
    ERROR_VARIABLE _NUMPY_ERROR_VALUE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT _NUMPY_SEARCH_SUCCESS MATCHES 0)
    if(NumPy_FIND_REQUIRED)
        message(FATAL_ERROR
            "NumPy import failure:\n${_NUMPY_ERROR_VALUE}")
    endif()
    set(NUMPY_FOUND FALSE)
endif()

# Convert the process output into a list
string(REGEX REPLACE ";" "\\\\;" _NUMPY_VALUES ${_NUMPY_VALUES})
string(REGEX REPLACE "\n" ";" _NUMPY_VALUES ${_NUMPY_VALUES})
list(GET _NUMPY_VALUES 0 NUMPY_VERSION)
list(GET _NUMPY_VALUES 1 NUMPY_INCLUDE_DIRS)

# Make sure all directory separators are '/'
string(REGEX REPLACE "\\\\" "/" NUMPY_INCLUDE_DIRS ${NUMPY_INCLUDE_DIRS})

# Get the major and minor version numbers
string(REGEX REPLACE "\\." ";" _NUMPY_VERSION_LIST ${NUMPY_VERSION})
list(GET _NUMPY_VERSION_LIST 0 NUMPY_VERSION_MAJOR)
list(GET _NUMPY_VERSION_LIST 1 NUMPY_VERSION_MINOR)

find_package_message(NUMPY
    "Found NumPy ${NUMPY_VERSION}: ${NUMPY_INCLUDE_DIRS}"
    "${NUMPY_INCLUDE_DIRS}${NUMPY_VERSION}")

set(NUMPY_FOUND TRUE)