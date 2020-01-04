#include "integrator/IIntegrator.h"
#include "material/IMaterial.h"
#include "mesh/MeshBase.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"
#include "spectral/SpectrumDescriptor.h"

#include "Environment.h"
#include "SceneLoader.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_environment(py::module& m)
{
	py::class_<Environment, std::shared_ptr<Environment>>(m, "Environment")
		.def(py::init<std::wstring, const std::shared_ptr<SpectrumDescriptor>&, std::wstring>())
		.def("getSpectrum", &Environment::getSpectrum)
		.def("hasSpectrum", &Environment::hasSpectrum)
		.def("addSpectrum", &Environment::addSpectrum)
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
		.def_property_readonly("spectrumDescriptor", &Environment::spectrumDescriptor);

	py::class_<SceneLoader>(m, "SceneLoader")
		.def_static("loadFromString", &SceneLoader::loadFromString)
		.def_static("loadFromFile", &SceneLoader::loadFromFile);
}
} // namespace PRPY