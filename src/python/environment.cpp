#include "integrator/IIntegrator.h"
#include "material/IMaterial.h"
#include "mesh/MeshBase.h"
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
		.def(py::init<std::wstring, std::wstring>())
		.def("getMaterial", &Environment::getMaterial)
		.def("hasMaterial", &Environment::hasMaterial)
		.def("addMaterial", &Environment::addMaterial)
		.def_property_readonly("materialCount", &Environment::materialCount)
		//.def("getMesh", &Environment::getMesh)
		//.def("hasMesh", &Environment::hasMesh)
		//.def("addMesh", &Environment::addMesh)
		// TODO: Sockets
		.def("dumpInformation", &Environment::dumpInformation)
		.def("setup", &Environment::setup)
		.def("save", &Environment::save)
		.def("createSelectedIntegrator", &Environment::createSelectedIntegrator)
		.def("createRenderFactory", &Environment::createRenderFactory)
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
		.def_readwrite("PluginPath", &SceneLoader::LoadOptions::PluginPath)
		.def_readwrite("WorkingDir", &SceneLoader::LoadOptions::WorkingDir)
		.def_readwrite("CacheMode", &SceneLoader::LoadOptions::CacheMode);
}
} // namespace PRPY