set(SRC
    skysun/ElevationAzimuth.h
    skysun/SkyModel.cpp
    skysun/SkyModel.h
    skysun/SkySunConfig.h
    skysun/SunLocation.cpp
    skysun/SunLocation.h
    skysun/SunRadiance.cpp
    skysun/SunRadiance.h
    skysun/model/ArHosekSkyModel.cpp
    skysun/model/ArHosekSkyModel.h
    skysun/model/ArHosekSkyModelData_CIEXYZ.h
    skysun/model/ArHosekSkyModelData_RGB.h
    skysun/model/ArHosekSkyModelData_Spectral.h)

add_library(pr_lib_skysun STATIC ${SRC})
target_link_libraries(pr_lib_skysun PUBLIC pr_lib_loader)
# Calling convention is skysun/
target_include_directories(pr_lib_skysun PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_PREFIX}/>)
set_property(TARGET pr_lib_skysun PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(pr_lib_skysun PROPERTIES EXPORT_NAME skysun)
install(TARGETS pr_lib_skysun COMPONENT development EXPORT PearRay-targets)
installHeaderFiles(${SRC})
