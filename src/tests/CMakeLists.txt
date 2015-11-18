CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

message(STATUS "Enabling Testing")
enable_testing()

set(PRT_Root ${PR_Root}/tests)

set(LIBS ${PR_TARGET})

FUNCTION(PUSH_TEST name files)
	add_executable(pr_test_${name} ${files})
	target_link_libraries(pr_test_${name} ${LIBS})
	set_target_properties(pr_test_${name} PROPERTIES VERSION ${PR_Version})
	set_target_properties(pr_test_${name} PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")
	add_test(NAME ${name} COMMAND pr_test_${name})
ENDFUNCTION(PUSH_TEST)

PUSH_TEST(spectrum spectrum.cpp)