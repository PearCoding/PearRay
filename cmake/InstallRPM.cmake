# RPM Stuff
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_MAIN_COMPONENT runtime)

set(CPACK_RPM_runtime_PACKAGE_DESCRIPTION "Runtime environment for the ${PROJECT_NAME} raytracer.")
set(CPACK_RPM_runtime_PACKAGE_REQUIRES "OpenImageIO >= 2.1, tbb")

set(CPACK_RPM_viewer_PACKAGE_DESCRIPTION "Viewers for the ${PROJECT_NAME} raytracer.")
set(CPACK_RPM_viewer_PACKAGE_REQUIRES "OpenImageIO >= 2.1, tbb, qt5-qtbase-gui, qt5-qtcharts")

set(CPACK_RPM_development_PACKAGE_DESCRIPTION "Development environment for the ${PROJECT_NAME} raytracer.")
set(CPACK_RPM_development_PACKAGE_REQUIRES "eigen3-devel, OpenImageIO-devel >= 2.1, tbb-devel")

set(CPACK_RPM_documentation_PACKAGE_DESCRIPTION "API Documentation for the ${PROJECT_NAME} raytracer.")
