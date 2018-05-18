#include "geometry/TriMesh.h"
#include "material/Material.h"

#include "Environment.h"
#include "SceneLoader.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_environment(py::module& m)
{
	py::class_<Environment, std::shared_ptr<Environment>>(m, "Environment")
		.def(py::init<const std::shared_ptr<SpectrumDescriptor>&, std::string>())
		.def_property_readonly("sceneFactory", (SceneFactory & (Environment::*)()) & Environment::sceneFactory)
		.def("getSpectrum", &Environment::getSpectrum)
		.def("hasSpectrum", &Environment::hasSpectrum)
		.def("addSpectrum", &Environment::addSpectrum)
		.def("getMaterial", &Environment::getMaterial)
		.def("hasMaterial", &Environment::hasMaterial)
		.def("addMaterial", &Environment::addMaterial)
		.def_property_readonly("materialCount", &Environment::materialCount)
		.def("getMesh", &Environment::getMesh)
		.def("hasMesh", &Environment::hasMesh)
		.def("addMesh", &Environment::addMesh)
		.def("addShaderOutput",
			 (void (Environment::*)(const std::string&, const std::shared_ptr<PR::ScalarShaderOutput>&)) & Environment::addShaderOutput)
		.def("addShaderOutput",
			 (void (Environment::*)(const std::string&, const std::shared_ptr<PR::SpectrumShaderOutput>&)) & Environment::addShaderOutput)
		.def("addShaderOutput",
			 (void (Environment::*)(const std::string&, const std::shared_ptr<PR::VectorShaderOutput>&)) & Environment::addShaderOutput)
		.def("getScalarShaderOutput", &Environment::getScalarShaderOutput)
		.def("hasScalarShaderOutput", &Environment::hasScalarShaderOutput)
		.def("getSpectrumShaderOutput", &Environment::getSpectrumShaderOutput)
		.def("hasSpectrumShaderOutput", &Environment::hasSpectrumShaderOutput)
		.def("getVectorShaderOutput", &Environment::getVectorShaderOutput)
		.def("hasVectorShaderOutput", &Environment::hasVectorShaderOutput)
		.def_property("renderWidth", &Environment::renderWidth, &Environment::setRenderWidth)
		.def_property("renderHeight", &Environment::renderHeight, &Environment::setRenderHeight)
		.def("setCrop", &Environment::setCrop)
		.def_property_readonly("cropMinX", &Environment::cropMinX)
		.def_property_readonly("cropMaxX", &Environment::cropMaxX)
		.def_property_readonly("cropMinY", &Environment::cropMinY)
		.def_property_readonly("cropMaxY", &Environment::cropMaxY)
		.def_property_readonly("registry", &Environment::registry)
		// TODO
		.def("dumpInformation", &Environment::dumpInformation)
		.def("setup", &Environment::setup)
		.def("save", &Environment::save);

	py::class_<SceneLoader>(m, "SceneLoader")
		.def_static("loadFromString", &SceneLoader::loadFromString)
		.def_static("loadFromFile", &SceneLoader::loadFromFile);
}
}