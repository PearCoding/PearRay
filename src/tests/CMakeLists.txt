CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

message(STATUS "Enabling Testing")
enable_testing()

set(PRT_Root ${PR_Root}/tests)
set(PRT_Pre ${PRT_Root}/Test.h)

set(LIBS ${PR_TARGET})

FUNCTION(PUSH_TEST name files)
	list(APPEND files ${PRT_Pre})
	add_executable(pr_test_${name} ${files})
	target_link_libraries(pr_test_${name} ${LIBS})
	set_target_properties(pr_test_${name} PROPERTIES VERSION ${PR_Version})
	set_target_properties(pr_test_${name} PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")
	add_test(NAME ${name} COMMAND pr_test_${name})
ENDFUNCTION(PUSH_TEST)

PUSH_TEST(spectrum spectrum.cpp)
PUSH_TEST(plane plane.cpp)
PUSH_TEST(entity entity.cpp)
PUSH_TEST(boundingbox boundingbox.cpp)
PUSH_TEST(projection projection.cpp)
PUSH_TEST(brdf brdf.cpp)
PUSH_TEST(sphere sphere.cpp)