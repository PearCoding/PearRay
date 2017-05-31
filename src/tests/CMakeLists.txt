CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

if(VERBOSE)
	message(STATUS "Enabling Testing")
endif()

include_directories(../utils)

set(PRT_Pre Test.h testscene.inl)

set(LIBS pr_lib pr_lib_utils)

FUNCTION(PUSH_TEST2 name files)
	list(APPEND files ${PRT_Pre})
	add_executable(pr_test_${name} ${files})
	target_link_libraries(pr_test_${name} ${LIBS})
	set_target_properties(pr_test_${name} PROPERTIES VERSION ${PR_Version})
	target_compile_definitions(pr_test_${name} PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
ENDFUNCTION(PUSH_TEST2)

FUNCTION(PUSH_TEST name files)
	PUSH_TEST2(${name} ${files})
	add_test(NAME ${name} COMMAND pr_test_${name})
ENDFUNCTION(PUSH_TEST)

PUSH_TEST(spectrum spectrum.cpp)
PUSH_TEST(plane plane.cpp)
PUSH_TEST(entity entity.cpp)
PUSH_TEST(boundingbox boundingbox.cpp)
PUSH_TEST(projection projection.cpp)
PUSH_TEST(brdf brdf.cpp)
PUSH_TEST(sphere sphere.cpp)
PUSH_TEST(generator generator.cpp)
PUSH_TEST(triangle triangle.cpp)
PUSH_TEST(fresnel fresnel.cpp)
PUSH_TEST2(inteqen inteqen.cpp)# Disable it on make test
PUSH_TEST(random random.cpp)
PUSH_TEST2(seed seed.cpp)