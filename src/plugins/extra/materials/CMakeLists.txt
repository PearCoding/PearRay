if(PR_EXTRA_RGL_BRDF)
    include(ExternalProject)
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