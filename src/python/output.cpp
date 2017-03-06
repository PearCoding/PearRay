#include <boost/python.hpp>
#include "shader/ShaderClosure.h"
#include "renderer/OutputMap.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    void setup_output()
    {
        // TODO: Add ptr() for bpy::object?
        bpy::class_<Output3D, boost::noncopyable>("Output3D", bpy::init<RenderContext*, const PM::vec3&, bpy::optional<bool> >())
            .add_property("neverClear", &Output3D::isNeverCleared, &Output3D::setNeverClear)
            .def("setFragment", &Output3D::setFragment)
            .def("getFragment", &Output3D::getFragment, bpy::return_value_policy<bpy::copy_const_reference >())
            .def("setFragmentBounded", &Output3D::setFragmentBounded)
            .def("getFragmentBounded", &Output3D::getFragmentBounded, bpy::return_value_policy<bpy::copy_const_reference >())
            .def("clear", &Output3D::clear)
            .def("fill", &Output3D::fill)
        ;

        bpy::class_<Output1D, boost::noncopyable>("Output1D", bpy::init<RenderContext*, bpy::optional<float,bool> >())
            .add_property("neverClear", &Output1D::isNeverCleared, &Output1D::setNeverClear)
            .def("setFragment", &Output1D::setFragment)
            .def("getFragment", &Output1D::getFragment, bpy::return_value_policy<bpy::copy_const_reference >())
            .def("setFragmentBounded", &Output1D::setFragmentBounded)
            .def("getFragmentBounded", &Output1D::getFragmentBounded, bpy::return_value_policy<bpy::copy_const_reference >())
            .def("clear", &Output1D::clear)
            .def("fill", &Output1D::fill)
        ;

        bpy::class_<OutputCounter, boost::noncopyable>("OutputCounter", bpy::init<RenderContext*, bpy::optional<uint64,bool> >())
            .add_property("neverClear", &OutputCounter::isNeverCleared, &OutputCounter::setNeverClear)
            .def("setFragment", &OutputCounter::setFragment)
            .def("getFragment", &OutputCounter::getFragment, bpy::return_value_policy<bpy::copy_const_reference >())
            .def("setFragmentBounded", &OutputCounter::setFragmentBounded)
            .def("getFragmentBounded", &OutputCounter::getFragmentBounded, bpy::return_value_policy<bpy::copy_const_reference >())
            .def("clear", &OutputCounter::clear)
            .def("fill", &OutputCounter::fill)
        ;

        bpy::class_<OutputSpectral, boost::noncopyable>("OutputSpectral", bpy::init<RenderContext*, bpy::optional<Spectrum,bool> >())
            .add_property("neverClear", &OutputSpectral::isNeverCleared, &OutputSpectral::setNeverClear)
            .def("setFragment", &OutputSpectral::setFragment)
            .def("getFragment", &OutputSpectral::getFragment, bpy::return_internal_reference<>())
            .def("setFragmentBounded", &OutputSpectral::setFragmentBounded)
            .def("getFragmentBounded", &OutputSpectral::getFragmentBounded, bpy::return_internal_reference<>())
            .def("clear", &OutputSpectral::clear)
            .def("fill", &OutputSpectral::fill)
        ;

        bpy::register_ptr_to_python<std::shared_ptr<Output3D> >();
        bpy::register_ptr_to_python<std::shared_ptr<Output1D> >();
        bpy::register_ptr_to_python<std::shared_ptr<OutputCounter> >();
        bpy::register_ptr_to_python<std::shared_ptr<OutputSpectral> >();

        { bpy::scope scope = bpy::class_<OutputMap, boost::noncopyable>("OutputMap", bpy::no_init)
            .def("clear", &OutputMap::clear)
            .def("pushFragment", &OutputMap::pushFragment)
            .def("fragment", &OutputMap::getFragment)
            .def("setSampleCount", &OutputMap::setSampleCount)
            .def("sampleCount", &OutputMap::getSampleCount)
            .def("isPixelFinished", &OutputMap::isPixelFinished)
            .add_property("finishedPixelCount", &OutputMap::finishedPixelCount)
            .def("channel",
                (const std::shared_ptr<Output1D>&(OutputMap::*)(OutputMap::Variable1D) const)&OutputMap::getChannel,
                bpy::return_value_policy<bpy::copy_const_reference >())
            .def("channel",
                (const std::shared_ptr<Output3D>&(OutputMap::*)(OutputMap::Variable3D) const)&OutputMap::getChannel,
                bpy::return_value_policy<bpy::copy_const_reference >())
            .def("channel",
                (const std::shared_ptr<OutputCounter>&(OutputMap::*)(OutputMap::VariableCounter) const)&OutputMap::getChannel,
                bpy::return_value_policy<bpy::copy_const_reference >())
            .add_property("spectral", bpy::make_function(&OutputMap::getSpectralChannel, bpy::return_internal_reference<>()))
            .def("registerChannel",
                (void (OutputMap::*)(OutputMap::Variable1D, const std::shared_ptr<Output1D>&))&OutputMap::registerChannel)
            .def("registerChannel",
                (void (OutputMap::*)(OutputMap::Variable3D, const std::shared_ptr<Output3D>&))&OutputMap::registerChannel)
            .def("registerChannel",
                (void (OutputMap::*)(OutputMap::VariableCounter, const std::shared_ptr<OutputCounter>&))&OutputMap::registerChannel)
            .def("registerCustomChannel",
                (void (OutputMap::*)(const std::string&, const std::shared_ptr<Output1D>&))&OutputMap::registerCustomChannel)
            .def("registerCustomChannel",
                (void (OutputMap::*)(const std::string&, const std::shared_ptr<Output3D>&))&OutputMap::registerCustomChannel)
            .def("registerCustomChannel",
                (void (OutputMap::*)(const std::string&, const std::shared_ptr<OutputCounter>&))&OutputMap::registerCustomChannel)
            .def("registerCustomChannel",
                (void (OutputMap::*)(const std::string&, const std::shared_ptr<OutputSpectral>&))&OutputMap::registerCustomChannel)
        ;

        bpy::enum_<OutputMap::Variable3D>("Variable3D")
        .value("Position", OutputMap::V_Position)
        .value("Normal", OutputMap::V_Normal)
        .value("NormalG", OutputMap::V_NormalG)
        .value("Tangent", OutputMap::V_Tangent)
        .value("Bitangent", OutputMap::V_Bitangent)
        .value("View", OutputMap::V_View)
        .value("UVW", OutputMap::V_UVW)
        .value("DPDU", OutputMap::V_DPDU)
        .value("DPDV", OutputMap::V_DPDV)
        .value("DPDW", OutputMap::V_DPDW)
        .value("DPDX", OutputMap::V_DPDX)
        .value("DPDY", OutputMap::V_DPDY)
        .value("DPDZ", OutputMap::V_DPDZ)
        .value("DPDT", OutputMap::V_DPDT)
        ;

        bpy::enum_<OutputMap::Variable1D>("Variable1D")
        .value("Depth", OutputMap::V_Depth)
        .value("Time", OutputMap::V_Time)
        .value("Material", OutputMap::V_Material)
        ;

        bpy::enum_<OutputMap::VariableCounter>("VariableCounter")
        .value("ID", OutputMap::V_ID)
        .value("Samples", OutputMap::V_Samples)
        ;
        }
    }
}