include(ExternalProject)
include(FetchContent)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if(PR_EXTRA_RGL_BRDF)
    ExternalProject_Add(rgl_brdf_download
        GIT_REPOSITORY    git@github.com:rgl-epfl/brdf-loader.git
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        UPDATE_DISCONNECTED ON
    )
    ExternalProject_Get_Property(rgl_brdf_download source_dir)
    PR_ADD_PLUGIN(mat_rgl_measured CPP rgl_measured.cpp INCLUDES ${source_dir})
endif()

if(PR_EXTRA_DAYLIGHT)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
    FetchContent_Declare(pugixml
        GIT_REPOSITORY  git@github.com:zeux/pugixml.git
    )

    set(BUILD_TESTS OFF)
    set(BUILD_SHARED_AND_STATIC_LIBS OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(USE_POSTFIX ON)
    FetchContent_MakeAvailable(pugixml)

    PR_ADD_PLUGIN(mat_klems_measured CPP klems_measured.cpp LIBRARIES pugixml)
endif()