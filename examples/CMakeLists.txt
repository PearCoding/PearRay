
file(GLOB EXAMPLE_FILES "*.prc")

set(EXAMPLE_TARGETS )

# Ignore some scenes as they are to complex and take too long.
# They still can be run explicitly however
set(IGNORE_IN_ALL complex)

foreach(EXAMPLE_FILE ${EXAMPLE_FILES})
	get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
	set(_target_name pr_run_example_${EXAMPLE_NAME})
	set(_wrk_dir ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE_NAME} )
	file(MAKE_DIRECTORY ${_wrk_dir})
	add_custom_target(${_target_name} 
		COMMAND pearray ${EXAMPLE_FILE}
		DEPENDS pearray
		WORKING_DIRECTORY ${_wrk_dir}
		COMMENT "Rendering ${EXAMPLE_FILE}"
		USES_TERMINAL
		)
	if(NOT EXAMPLE_NAME IN_LIST IGNORE_IN_ALL)
		list(APPEND EXAMPLE_TARGETS ${_target_name})
	endif()
endforeach()

add_custom_target(pr_run_all_examples)
add_dependencies(pr_run_all_examples ${EXAMPLE_TARGETS})