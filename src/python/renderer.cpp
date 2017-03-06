#include <boost/python.hpp>
#include "renderer/RenderFactory.h"
#include "renderer/RenderContext.h"
#include "renderer/OutputMap.h"
#include "renderer/RenderTile.h"
#include "scene/Scene.h"
#include "camera/Camera.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    class RenderContextWrap : public RenderContext, public bpy::wrapper<RenderContext>
    {
    public:
        inline void start_Py(uint32 tx, uint32 ty)
        {
            start(tx, ty);
        }
    };

    void setup_renderer()
    {
        bpy::class_<RenderFactory>("RenderFactory",
            bpy::init<uint32, uint32, const Scene&, const std::string&, bpy::optional<bool> >())
            .add_property("fullWidth", &RenderFactory::fullWidth, &RenderFactory::setFullWidth)
            .add_property("fullHeight", &RenderFactory::fullHeight, &RenderFactory::setFullHeight)
            .add_property("cropWidth", &RenderFactory::cropWidth)
            .add_property("cropHeight", &RenderFactory::cropHeight)
            .add_property("cropOffsetX", &RenderFactory::cropOffsetX)
            .add_property("cropOffsetY", &RenderFactory::cropOffsetY)
            .def("create", (std::shared_ptr<RenderContext> (RenderFactory::*)() const)&RenderFactory::create)
            .def("create", (std::shared_ptr<RenderContext> (RenderFactory::*)(uint32,uint32,uint32) const)&RenderFactory::create)
            .add_property("settings",
                bpy::make_function((RenderSettings& (RenderFactory::*)())&RenderFactory::settings, bpy::return_internal_reference<>()),
                &RenderFactory::setSettings)
            .add_property("workingDir", &RenderFactory::workingDir, &RenderFactory::setWorkingDir)
            .add_property("scene", bpy::make_function(&RenderFactory::scene, bpy::return_internal_reference<>()))
        ;

        bpy::class_<RenderContextWrap, boost::noncopyable>("RenderContext", bpy::no_init)
            .add_property("width", &RenderContext::width)
            .add_property("height", &RenderContext::height)
            .add_property("fullWidth", &RenderContext::fullWidth)
            .add_property("fullHeight", &RenderContext::fullHeight)
            .add_property("offsetX", &RenderContext::offsetX)
            .add_property("offsetY", &RenderContext::offsetY)
            .add_property("index", &RenderContext::index)
            .add_property("threads", &RenderContext::threads)
            .add_property("finished", &RenderContext::isFinished)
            .add_property("currentPass", &RenderContext::currentPass)
            .add_property("currentTiles", &RenderContext::currentTiles)
            .add_property("settings",
                bpy::make_function(&RenderContext::settings, bpy::return_internal_reference<>()))
            .add_property("lights",
                bpy::make_function(&RenderContext::lights, bpy::return_internal_reference<>()))
            .add_property("workingDir", &RenderContext::workingDir)
            .add_property("scene", bpy::make_function(&RenderContext::scene, bpy::return_internal_reference<>()))
            .add_property("output", bpy::make_function(&RenderContext::output, bpy::return_internal_reference<>()))
            .add_property("status", &RenderContext::status)

            .def("start", (void (RenderContext::*)(uint32, uint32, uint32))&RenderContext::start)
            .def("start", (void (RenderContext::*)(uint32, uint32))&RenderContextWrap::start_Py)
            .def("stop", &RenderContext::stop)
            .def("waitForFinish", &RenderContext::waitForFinish)
        ;

        bpy::class_<RenderTile>("RenderTile", bpy::no_init)
            .add_property("sx", &RenderTile::sx)
            .add_property("sy", &RenderTile::sy)
            .add_property("ex", &RenderTile::ex)
            .add_property("ey", &RenderTile::ey)
            .add_property("samplesRendered", &RenderTile::samplesRendered)
            .add_property("working", &RenderTile::isWorking)
        ;

        bpy::register_ptr_to_python<std::shared_ptr<RenderContext> >();
    }
}