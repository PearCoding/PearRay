#include <sstream>
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>

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
    void setup_material();
    void setup_settings();
    void setup_output();
    void setup_scene();
    void setup_geometry();
    void setup_camera();
    void setup_renderentities();
    void setup_light();
    void setup_sampler();
    void setup_vec();
    void setup_mat();
    void setup_status();
    void setup_renderer();
}
//----------
BOOST_PYTHON_MODULE(pypearray)
{
    bpy::numeric::array::set_module_and_type("numpy", "ndarray");

    bpy::def("version", version);

    PRPY::setup_vec();
    PRPY::setup_mat();
    PRPY::setup_logger();
    PRPY::setup_spectral();
    PRPY::setup_ray();
    PRPY::setup_sampler();
    PRPY::setup_geometry();
    PRPY::setup_shader();
    PRPY::setup_material();
    PRPY::setup_entity();
    PRPY::setup_camera();
    PRPY::setup_renderentities();
    PRPY::setup_light();
    PRPY::setup_scene();
    PRPY::setup_settings();
    PRPY::setup_output();
    PRPY::setup_status();
    PRPY::setup_renderer();
}