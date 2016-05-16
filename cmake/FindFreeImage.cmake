# Locate Free Image
# This module defines
#  FREEIMAGE_FOUND, if false, do not try to link to FreeImage
#  FREEIMAGE_LIBRARIES
#  FREEIMAGE_INCLUDE_DIRS
#
# Note that the expected include convention is
#  #include "FreeImage.h"

find_package(PackageHandleStandardArgs)

find_path(FREEIMAGE_INCLUDE_DIR FreeImage.h
  HINTS
    ENV FREEIMAGE_DIR
  PATH_SUFFIXES include/FreeImage include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

find_library(FREEIMAGE_LIBRARY
  NAMES FreeImage
  HINTS
    ENV FREEIMAGE_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeImage
                                  REQUIRED_VARS FREEIMAGE_LIBRARY FREEIMAGE_INCLUDE_DIR)

mark_as_advanced(FREEIMAGE_INCLUDE_DIR FREEIMAGE_LIBRARY LUA_MATH_LIBRARY)

