#
# The MIT License (MIT)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Copyright (C) 2014 Joakim Söderberg <joakim.soderberg@gmail.com>
# Copyright (C) 2021 Ömercan Yazici <omercan@pearcoding.eu>
#

set(_CMAKE_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}) # must be outside coveralls_setup() to get correct path

function(coveralls_setup)

	if (ARGC GREATER 2)
		set(_CMAKE_SCRIPT_PATH ${ARGN})
		message(STATUS "Coveralls: Using alternate CMake script dir: ${_CMAKE_SCRIPT_PATH}")
	endif()

	if (NOT EXISTS "${_CMAKE_SCRIPT_PATH}/CoverallsClear.cmake")
		message(FATAL_ERROR "Coveralls: Missing ${_CMAKE_SCRIPT_PATH}/CoverallsClear.cmake")
	endif()

	#message("Coverage sources: ${COVERAGE_SRCS}")
	set(COVERALLS_CMD cpp-coveralls)

	add_custom_target(coveralls
		# Zero the coverage counters.
		COMMAND ${CMAKE_COMMAND} -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}" -P "${_CMAKE_SCRIPT_PATH}/CoverallsClear.cmake"

		# Run regress tests.
		COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure || true

		# Generate Gcov and upload
		COMMAND ${COVERALLS_CMD} --root ${PROJECT_SOURCE_DIR} --build-root ${PROJECT_BINARY_DIR} 
			--include "${PROJECT_SOURCE_DIR}/src/base" --include "${PROJECT_SOURCE_DIR}/src/core"  --include "${PROJECT_SOURCE_DIR}/src/loader" 
			--gcov-options '\\-lp' -x ".h" -x ".inl" -x ".cpp"

		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMENT "Updating coveralls..."
		)

endfunction()

macro(coveralls_turn_on_coverage)
	if(NOT (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		AND (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
		AND (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang"))
		message(FATAL_ERROR "Coveralls: Compiler ${CMAKE_C_COMPILER_ID} is not GNU gcc! Aborting... You can set this on the command line using CC=/usr/bin/gcc CXX=/usr/bin/g++ cmake <options> ..")
	endif()

	if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		message(FATAL_ERROR "Coveralls: Code coverage results with an optimised (non-Debug) build may be misleading! Add -DCMAKE_BUILD_TYPE=Debug")
	endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage -coverage")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage -coverage")
endmacro()



