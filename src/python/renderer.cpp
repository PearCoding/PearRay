#include "camera/Camera.h"
#include "renderer/OutputMap.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderTile.h"

#include "entity/RenderEntity.h"
#include "scene/Scene.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

void setup_renderer(py::module& m)
{
	py::class_<RenderFactory>(m, "RenderFactory")
		.def(py::init<const std::shared_ptr<SpectrumDescriptor>&, const std::shared_ptr<Scene>&, const std::shared_ptr<Registry>&, const std::string&>())
		.def("create", (std::shared_ptr<RenderContext>(RenderFactory::*)() const) & RenderFactory::create)
		.def("create", (std::shared_ptr<RenderContext>(RenderFactory::*)(uint32, uint32, uint32) const) & RenderFactory::create)
		.def_property("workingDir", &RenderFactory::workingDir, &RenderFactory::setWorkingDir)
		.def_property_readonly("scene", &RenderFactory::scene)
		.def_property_readonly("registry", &RenderFactory::registry);

	py::class_<RenderContext, std::shared_ptr<RenderContext>>(m, "RenderContext")
		.def_property_readonly("width", &RenderContext::width)
		.def_property_readonly("height", &RenderContext::height)
		.def_property_readonly("offsetX", &RenderContext::offsetX)
		.def_property_readonly("offsetY", &RenderContext::offsetY)
		.def_property_readonly("index", &RenderContext::index)
		.def_property_readonly("threads", &RenderContext::threads)
		.def_property_readonly("finished", &RenderContext::isFinished)
		.def_property_readonly("currentPass", &RenderContext::currentPass)
		.def_property_readonly("currentTiles", &RenderContext::currentTiles)
		.def_property_readonly("settings", &RenderContext::settings)
		.def_property_readonly("registry", &RenderContext::registry)
		.def_property_readonly("lights", &RenderContext::lights)
		.def_property_readonly("workingDir", &RenderContext::workingDir)
		.def_property_readonly("scene", &RenderContext::scene)
		.def_property_readonly("output", &RenderContext::output, py::return_value_policy::reference)
		.def_property_readonly("status", &RenderContext::status)

		.def("start", (void (RenderContext::*)(uint32, uint32, uint32)) & RenderContext::start)
		.def("start", (void (RenderContext::*)(uint32, uint32)) & RenderContext::start)
		.def("stop", &RenderContext::stop)
		.def("waitForFinish", &RenderContext::waitForFinish);

	py::class_<RenderTile>(m, "RenderTile")
		.def_property_readonly("sx", &RenderTile::sx)
		.def_property_readonly("sy", &RenderTile::sy)
		.def_property_readonly("ex", &RenderTile::ex)
		.def_property_readonly("ey", &RenderTile::ey)
		.def_property_readonly("samplesRendered", &RenderTile::samplesRendered)
		.def_property_readonly("working", &RenderTile::isWorking);
}
}