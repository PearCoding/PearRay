add_library(pr_lib_skysun STATIC 
    skysun/SkyModel.cpp
    skysun/SkyModel.h
    skysun/SkySunConfig.h
    skysun/SunLocation.cpp
    skysun/SunLocation.h
    skysun/skymodel/ArHosekSkyModel.cpp
    skysun/skymodel/ArHosekSkyModel.h
    skysun/skymodel/ArHosekSkyModelData_CIEXYZ.h
    skysun/skymodel/ArHosekSkyModelData_RGB.h
    skysun/skymodel/ArHosekSkyModelData_Spectral.h)
target_link_libraries(pr_lib_skysun PUBLIC pr_lib_loader)
target_include_directories(pr_lib_skysun PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
set_property(TARGET pr_lib_skysun PROPERTY POSITION_INDEPENDENT_CODE ON)

add_subdirectory(entities)
add_subdirectory(node)