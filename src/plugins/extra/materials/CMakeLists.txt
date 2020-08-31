include(ExternalProject)

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
ExternalProject_Add(pugixml_download
    GIT_REPOSITORY   git@github:zeux/pugixml.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(pugixml_download source_dir)
    PR_ADD_PLUGIN(mat_klems_measured CPP klems_measured.cpp LIBRARIES pr_extra_pugixml)
endif()