# Call extractHeaderFiles(headers test.h test.cpp test.inl)
function(extractHeaderFiles LIST)
    set(data ${ARGN})
    list(FILTER data INCLUDE REGEX "(.*\.h$)|(.*\.hpp$)|(.*\.inl$)")
    set(${LIST} ${data} PARENT_SCOPE)
endfunction()

# Call installHeaderFiles(test.h test.cpp details/test.inl)
function(installHeaderFiles)
    extractHeaderFiles(headers ${ARGN})
    foreach(header ${headers})
        get_filename_component(dir ${header} DIRECTORY)
        install(FILES ${header} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${INCLUDE_PREFIX}/${dir}" COMPONENT development)
    endforeach()
endfunction()

# Set plugin install folder 
set(PR_PLUGINS_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/plugins CACHE PATH "Directory available plugins are installed to.")

# Setup cmake package
# TODO: This could be further optimized
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/PearRay-config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    NO_SET_AND_CHECK_MACRO
    PATH_VARS PR_PLUGINS_INSTALL_DIR)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    COMPATIBILITY AnyNewerVersion)
install(FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} COMPONENT development)

install(EXPORT PearRay-targets
        NAMESPACE PearRay::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} COMPONENT development)

# CPack stuff
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PearRay_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "${PearRay_VENDOR}")
set(CPACK_PACKAGE_DESCRIPTION "Experimental high accurate spectral path and ray tracer for research and data acquisition.")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CONTACT "https://pearcoding.eu/projects/PearRay")

SET(CPACK_COMPONENTS_ALL runtime viewer development documentation)
set(CPACK_COMPONENT_viewer_DEPENDS runtime)
set(CPACK_COMPONENT_development_DEPENDS runtime)

include(InstallArchive)
include(InstallBSD)
include(InstallDeb)
include(InstallRPM)

include(CPack)

cpack_add_component_group(runtime DISPLAY_NAME "PearRay runtime files")
cpack_add_component_group(viewer PARENT_GROUP runtime DISPLAY_NAME "PearRay viewers")
cpack_add_component_group(development PARENT_GROUP runtime DISPLAY_NAME "PearRay development files")
cpack_add_component_group(documentation DISPLAY_NAME "PearRay documentation files")

cpack_add_component(runtime GROUP runtime)
cpack_add_component(viewer GROUP viewer)
cpack_add_component(development GROUP development)
cpack_add_component(documentation GROUP documentation)