if(CMAKE_VERSION VERSION_GREATER 3.6)
    # Add iwyu if available
    option(PR_USE_IWYU "Use include-what-you-use while building" OFF)
    IF(PR_USE_IWYU)
        find_program(IWYU_EXECUTABLE NAMES include-what-you-use iwyu)
        IF(IWYU_EXECUTABLE)
            MESSAGE(STATUS "Using IWYU ${IWYU_EXECUTABLE}")
            SET(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXECUTABLE})
        ENDIF()
    ENDIF()

    # Add clang-tidy if available
    option(PR_USE_CLANG_TIDY "Use Clang-Tidy" OFF)
    option(PR_USE_CLANG_TIDY_FIX "Let Clang-Tidy fix" OFF)

    if(PR_USE_CLANG_TIDY)
        find_program(
            CLANG_TIDY_EXECUTABLE
            NAMES "clang-tidy"
            DOC "Path to clang-tidy executable"
        )

        if(CLANG_TIDY_EXECUTABLE)
            MESSAGE(STATUS "Using Clang-Tidy ${CLANG_TIDY_EXECUTABLE}")
            if(PR_USE_CLANG_TIDY_FIX)
                set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}" "-fix")
            else()
                set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
            endif()
        endif()
    endif()
endif()

if(CMAKE_VERSION VERSION_GREATER 3.8)
    # Add cpplint if available
    option(PR_USE_CPPLINT "Do style check with cpplint." OFF)
    if(PR_USE_CPPLINT)
        find_program(
            CPPLINT_EXECUTABLE
            NAMES "cpplint"
            DOC "Path to cpplint executable"
        )

        set(PR_CPPLINT_STYLE)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/braces,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/tab,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/line_length,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/comments,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-whitespace/indent,)
        #set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include_order,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/namespaces,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include_what_you_use,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-build/include,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-legal/copyright,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-readability/namespace,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-readability/todo,)
        set(PR_CPPLINT_STYLE ${PR_CPPLINT_STYLE}-runtime/references,)

        if(CPPLINT_EXECUTABLE)
            MESSAGE(STATUS "Using cpplint ${CPPLINT_EXECUTABLE}")
            set(CMAKE_CXX_CPPLINT "${CPPLINT_EXECUTABLE}"
                "--filter=${PR_CPPLINT_STYLE}"
                "--counting=detailed"
                "--extensions=cpp,h,inl"
                "--headers=h,inl")
        endif()
    endif()
endif()

if(CMAKE_VERSION VERSION_GREATER 3.10)
    # Add cppcheck if available
    option(PR_USE_CPPCHECK "Do checks with cppcheck." OFF)
    if(PR_USE_CPPCHECK)
        find_program(
            CPPCHECK_EXECUTABLE
            NAMES "cppcheck"
            DOC "Path to cppcheck executable"
        )

        if(CPPCHECK_EXECUTABLE)
            MESSAGE(STATUS "Using cppcheck ${CPPCHECK_EXECUTABLE}")
            if(WIN32)
                set(ARCH_ARGS "-DWIN32;-D_MSC_VER")
            else()
                set(ARCH_ARGS "-Dlinux;-D__GNUC__")
            endif()
            set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXECUTABLE};${ARCH_ARGS};--suppress=preprocessorErrorDirective")
        endif()
    endif()
endif()