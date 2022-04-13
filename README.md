:exclamation: **This project is currently on hold in favour of my other renderer [Ignis](https://github.com/PearCoding/Ignis).** :exclamation:

# PearRay [![GitHub Tag](https://img.shields.io/github/tag/PearCoding/PearRay.svg)](https://github.com/PearCoding/PearRay/releases) [![Build Status](https://travis-ci.org/PearCoding/PearRay.svg?branch=master)](https://travis-ci.org/PearCoding/PearRay)

[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/PearCoding/PearRay/master/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/PearCoding/PearRay.svg)](https://github.com/PearCoding/PearRay/issues)
[![Coverage Status](https://coveralls.io/repos/github/PearCoding/PearRay/badge.svg?branch=master)](https://coveralls.io/github/PearCoding/PearRay?branch=master)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/PearCoding/PearRay.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/PearCoding/PearRay/alerts/)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/48a91c3c277d4aa4ae76ff940e4bcf07)](https://www.codacy.com/app/PearCoding/PearRay?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PearCoding/PearRay&amp;utm_campaign=Badge_Grade)\
![CMake 3.9](https://img.shields.io/badge/CMake-3.9+-green.svg)
![Language](https://img.shields.io/badge/language-c++-blue.svg)
![C++ Standard](https://img.shields.io/badge/std-c++17-blue.svg)
![GCC 7](https://img.shields.io/badge/GCC-7+-blue.svg)
![Clang 8](https://img.shields.io/badge/Clang-8+-blue.svg)
![VS 19](https://img.shields.io/badge/VS-19+-blue.svg)
![Language](https://img.shields.io/badge/language-Python-orange.svg)
![Python](https://img.shields.io/badge/Python-3.5+-orange.svg)

Experimental high accurate spectral path and ray tracer for research and data acquisition.

:exclamation: **This is experimental software. API changes regularly** :exclamation:

![Example render by PearRay. Modeled with Blender 2.80](examples/complex.jpeg)

## Blender Addon

There is an open source (still experimental) blender integration addon available [here](https://github.com/PearCoding/PearRay-Blender).

## Dependencies

- Eigen3 <http://eigen.tuxfamily.org>
- OpenImageIO <https://sites.google.com/site/openimageio/home>
- Intel® Embree 3 <https://www.embree.org/>
- Intel® Threading Building Blocks <https://www.threadingbuildingblocks.org/>

### Optional

- OpenSubdiv <https://github.com/PixarAnimationStudios/OpenSubdiv>
- Qt <https://www.qt.io/>

### Submodules

- cxxopts <https://github.com/jarro2783/cxxopts>
- DataLisp <https://github.com/PearCoding/DataLisp>
- pybind11 <https://github.com/pybind/pybind11>
- tinyobjloader <https://github.com/syoyo/tinyobjloader>

### Embedded

- PCG Random Number Generation, C++ Edition <https://github.com/imneme/pcg-cpp>

### Other

- pugixml <https://github.com/zeux/pugixml> (daylight plugins)
- brdf-loader <https://github.com/rgl-epfl/brdf-loader> (rgl-epfl measured material plugin)

## Wiki

See [Wiki](https://github.com/PearCoding/PearRay/wiki) for more information, examples and tutorials.
