This directory contains find files used by CMake and our build system.
Only files not in the official CMake modules are included.

Only exception to the rule above is FindBoost.cmake,
as the version provided by CMake 3.6 does not include the required component 'numpy'.
For more information see the original source:
https://gitlab.kitware.com/rleigh/cmake/raw/boost-1.64-numpy/Modules/FindBoost.cmake
or the issue:
https://gitlab.kitware.com/cmake/cmake/issues/16612

Anyone using Ubuntu or other debian distributions,
the provided boost version in the package manager did not provide the necessary libboost-numpy.so binary,
you have to build boost on your own. (Well I had to do it in Ubuntu 17.04)