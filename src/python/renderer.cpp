#include "buffer/OutputBuffer.h"
#include "entity/IEntity.h"
#include "integrator/IIntegrator.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderTile.h"
#include "scene/Scene.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_renderer(py::module& m)
{
	py::class_<RenderFactory, std::shared_ptr<RenderFactory>>(m, "RenderFactory")
		.def("create", (std::shared_ptr<RenderContext>(RenderFactory::*)(const std::shared_ptr<IIntegrator>&) const) & RenderFactory::create)
		.def("create", (std::shared_ptr<RenderContext>(RenderFactory::*)(const std::shared_ptr<IIntegrator>&, uint32, uint32, uint32) const) & RenderFactory::create);

	py::class_<RenderContext, std::shared_ptr<RenderContext>>(m, "RenderContext")
		.def_property_readonly("width", &RenderContext::width)
		.def_property_readonly("height", &RenderContext::height)
		.def_property_readonly("offsetX", &RenderContext::offsetX)
		.def_property_readonly("offsetY", &RenderContext::offsetY)
		.def_property_readonly("index", &RenderContext::index)
		.def_property_readonly("threads", &RenderContext::threads)
		.def_property_readonly("finished", &RenderContext::isFinished)
		.def_property_readonly("currentPass", &RenderContext::currentPass)
		//.def_property_readonly("currentTiles", &RenderContext::currentTiles)
		.def_property_readonly("settings", &RenderContext::settings)
		.def_property_readonly("lights", &RenderContext::lights)
		.def_property_readonly("scene", &RenderContext::scene)
		.def_property_readonly("output", &RenderContext::output)
		.def_property_readonly("status", &RenderContext::status)
		.def("start", &RenderContext::start, py::arg("tcx"),  py::arg("tcy"), py::arg("threads") = 0)
		.def("stop", &RenderContext::stop)
		.def("notifyEnd", &RenderContext::notifyEnd)
		.def("waitForFinish", &RenderContext::waitForFinish);

	/*py::class_<RenderTile>(m, "RenderTile")
		.def_property_readonly("sx", &RenderTile::sx)
		.def_property_readonly("sy", &RenderTile::sy)
		.def_property_readonly("ex", &RenderTile::ex)
		.def_property_readonly("ey", &RenderTile::ey)
		.def_property_readonly("samplesRendered", &RenderTile::samplesRendered)
		.def_property_readonly("working", &RenderTile::isWorking);*/
}
} // namespace PRPY