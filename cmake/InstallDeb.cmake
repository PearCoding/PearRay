# DEB Stuff
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_runtime_PACKAGE_DEPENDS "libopenimageio2.1, libtbb2")
set(CPACK_DEBIAN_runtime_PACKAGE_SECTION graphics)

set(CPACK_DEBIAN_viewer_PACKAGE_SECTION graphics)
set(CPACK_DEBIAN_viewer_PACKAGE_DEPENDS "libopenimageio2.1, libtbb2, libqt5gui5, libqt5charts5")

set(CPACK_DEBIAN_development_PACKAGE_SECTION devel)
set(CPACK_DEBIAN_development_PACKAGE_DEPENDS "libeigen3-dev, libopenimageio-dev (>= 2.1), libtbb-dev")

set(CPACK_DEBIAN_documentation_PACKAGE_SECTION doc)
set(CPACK_DEBIAN_documentation_PACKAGE_PRIORITY extra)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_runtime_PACKAGE_DEPENDS}, ${CPACK_DEBIAN_development_PACKAGE_DEPENDS}")
