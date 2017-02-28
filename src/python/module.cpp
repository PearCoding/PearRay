#include <sstream>
#include <boost/python.hpp>

#include "PR_Config.h"

using namespace PR;
namespace bpy = boost::python; 

bpy::tuple version()
{
    return bpy::make_tuple(PR_VERSION_MAJOR, PR_VERSION_MINOR);
}

namespace PRPY
{
    void setup_logger();
    void setup_spectral();
    void setup_entity();
    void setup_ray();
    void setup_shader();
}
//----------
BOOST_PYTHON_MODULE(pypearray)
{
    bpy::def("version", version);

    PRPY::setup_logger();
    PRPY::setup_spectral();
    PRPY::setup_ray();
    PRPY::setup_shader();
    PRPY::setup_entity();
}