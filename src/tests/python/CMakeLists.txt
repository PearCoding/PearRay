if(VERBOSE)
	message(STATUS "Enabling Python Testing")
endif()

find_package(PythonInterp)

if(NOT PYTHON_EXECUTABLE)
	message(WARNING "No Python executable found! Skipping Python test modules.")
	return()
endif()

FUNCTION(PUSH_PY_TEST name)
	add_test(NAME ${name}_py
		COMMAND ${PYTHON_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/run.py "$<TARGET_FILE:pypearray>" "${name}")
	set_tests_properties(${name}_py PROPERTIES DEPENDS pypearray)
ENDFUNCTION(PUSH_PY_TEST)

PUSH_PY_TEST(validity)
PUSH_PY_TEST(whitefurnance)