message(STATUS "Building python tests")

find_package(PythonInterp)

if(NOT PYTHON_EXECUTABLE)
	message(WARNING "No Python executable found! Skipping Python test modules.")
	return()
endif()

macro(push_test_py name)
	add_test(NAME ${name}_py
		COMMAND ${PYTHON_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/run.py "$<TARGET_FILE:pypearray>" "${name}")
	set_tests_properties(${name}_py PROPERTIES DEPENDS pypearray)
endmacro(push_test_py)

push_test_py(validity)
push_test_py(whitefurnance)