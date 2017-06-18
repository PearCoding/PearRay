if(VERBOSE)
	message(STATUS "Enabling Python Testing")
endif()

FUNCTION(PUSH_PY_TEST name)
	add_test(NAME ${name}_py 
		COMMAND ${PYTHON_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/run.py "$<TARGET_FILE:pypearray>" "${name}")
	set_tests_properties(${name}_py PROPERTIES DEPENDS pypearray)
ENDFUNCTION(PUSH_PY_TEST)

PUSH_PY_TEST(vector)