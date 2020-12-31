#include "integrator/IIntegrator.h"
#include "material/IMaterial.h"
#include "mesh/MeshBase.h"
#include "output/FrameOutputDevice.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"

#include "Environment.h"
#include "SceneLoader.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_environment(py::module& m)
{
	py::class_<OutputSaveOptions>(m, "OutputSaveOptions")
		.def(py::init<>())
		.def_readwrite("NameSuffix", &OutputSaveOptions::NameSuffix)
		//.def_readwrite("Image", &OutputSaveOptions::Image) /* TODO */
		.def_readwrite("Force", &OutputSaveOptions::Force);

	py::class_<Environment, std::shared_ptr<Environment>>(m, "Environment")
		.def_static("createRenderEnvironment", &Environment::createRenderEnvironment)
		.def_static("createQueryEnvironment", &Environment::createQueryEnvironment)
		.def("dumpInformation", &Environment::dumpInformation)
		.def("setup", &Environment::setup)
		.def("save", &Environment::save)
		.def("createSelectedIntegrator", &Environment::createSelectedIntegrator)
		.def("createRenderFactory", &Environment::createRenderFactory)
		.def("createAndAssignFrameOutputDevice", &Environment::createAndAssignFrameOutputDevice)
		.def_property(
			"renderSettings",
			[](Environment& env) { return env.renderSettings(); },
			[](Environment& env, const RenderSettings& st) { env.renderSettings() = st; },
			py::return_value_policy::reference_internal);

	auto sl = py::class_<SceneLoader>(m, "SceneLoader")
				  .def_static("loadFromString", &SceneLoader::loadFromString)
				  .def_static("loadFromFile", [](const std::wstring& path, const SceneLoader::LoadOptions& opts) {
					  return SceneLoader::loadFromFile(path, opts);
				  });

	py::class_<SceneLoader::LoadOptions>(sl, "LoadOptions")
		.def(py::init<>())
		.def_property(
			"PluginPath",
			[](SceneLoader::LoadOptions& ops) { return ops.PluginPath; },
			[](SceneLoader::LoadOptions& ops, const std::string& st) { ops.PluginPath = st; })
		.def_property(
			"WorkingDir",
			[](SceneLoader::LoadOptions& ops) { return ops.WorkingDir; },
			[](SceneLoader::LoadOptions& ops, const std::string& st) { ops.WorkingDir = st; })
		.def_readwrite("Progressive", &SceneLoader::LoadOptions::Progressive);
}
} // namespace PRPY