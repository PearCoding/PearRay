CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

if(VERBOSE)
	message(STATUS "Enabling Testing")
endif()

set(PRT_Pre Test.h)

set(LIBS pr_lib)

FUNCTION(PUSH_TEST name files)
	list(APPEND files ${PRT_Pre})
	add_executable(pr_test_${name} ${files})
	target_link_libraries(pr_test_${name} ${LIBS})
	set_target_properties(pr_test_${name} PROPERTIES VERSION ${PR_Version})
	target_compile_definitions(pr_test_${name} PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
	add_test(NAME ${name} COMMAND pr_test_${name})
ENDFUNCTION(PUSH_TEST)

PUSH_TEST(spectrum spectrum.cpp)
PUSH_TEST(plane plane.cpp)
PUSH_TEST(entity entity.cpp)
PUSH_TEST(boundingbox boundingbox.cpp)
PUSH_TEST(projection projection.cpp)
PUSH_TEST(brdf brdf.cpp)
PUSH_TEST(sphere sphere.cpp)