find_package(Git QUIET)
if(NOT GIT_FOUND OR NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    set(GIT_BRANCH "HEAD")
    set(GIT_REVISION "")
    return()
endif()

# Get the current working branch
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Update submodules as needed
option(GIT_SUBMODULE "Check submodules during build" ON)
if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    # Check always if a --recursive is really needed. All dependencies do not require them (yet)!
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --remote --init
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --remote --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

# Check at least one of the external modules exists if everything is fine
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/DataLisp/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()