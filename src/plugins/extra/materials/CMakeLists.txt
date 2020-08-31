include(ExternalProject)
include(FetchContent)

if(PR_EXTRA_RGL_BRDF)
    ExternalProject_Add(rgl_brdf_download
        GIT_REPOSITORY    git@github.com:rgl-epfl/brdf-loader.git
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    ExternalProject_Get_Property(rgl_brdf_download source_dir)
    PR_ADD_PLUGIN(mat_rgl_measured CPP rgl_measured.cpp INCLUDES ${source_dir})
endif()

if(PR_EXTRA_DAYLIGHT)
    FetchContent_Declare(pugixml
        GIT_REPOSITORY  git@github.com:zeux/pugixml.git
        GIT_SHALLOW     ON
    )

    set(pugixml_BUILD_TESTS OFF)
    set(pugixml_BUILD_SHARED_AND_STATIC_LIBS OFF)
    set(pugixml_BUILD_SHARED_LIBS OFF)
    set(pugixml_USE_POSTFIX ON)
    FetchContent_MakeAvailable(pugixml)

    PR_ADD_PLUGIN(mat_klems_measured CPP klems_measured.cpp LIBRARIES pugixml::static)
endif()