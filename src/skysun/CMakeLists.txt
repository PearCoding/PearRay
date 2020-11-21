add_library(pr_lib_skysun STATIC 
    ElevationAzimuth.h
    SkyModel.cpp
    SkyModel.h
    SkySunConfig.h
    SunLocation.cpp
    SunLocation.h
    SunRadiance.cpp
    SunRadiance.h
    skymodel/ArHosekSkyModel.cpp
    skymodel/ArHosekSkyModel.h
    skymodel/ArHosekSkyModelData_CIEXYZ.h
    skymodel/ArHosekSkyModelData_RGB.h
    skymodel/ArHosekSkyModelData_Spectral.h)
target_link_libraries(pr_lib_skysun PUBLIC pr_lib_loader)
# Calling convention is skysun/
target_include_directories(pr_lib_skysun PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/..)
set_property(TARGET pr_lib_skysun PROPERTY POSITION_INDEPENDENT_CODE ON)

install(TARGETS pr_lib_skysun COMPONENT development)