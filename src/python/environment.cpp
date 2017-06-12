#include <boost/python.hpp>

#include "geometry/TriMesh.h"
#include "material/Material.h"

#include "Environment.h"
#include "SceneLoader.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY {
void setup_environment()
{
	bpy::class_<Environment, std::shared_ptr<Environment>, boost::noncopyable>("Environment", bpy::init<std::string>())
		.add_property("scene",
					  bpy::make_function((Scene & (Environment::*)()) & Environment::scene, bpy::return_internal_reference<>()))
		.def("getSpectrum", &Environment::getSpectrum)
		.def("hasSpectrum", &Environment::hasSpectrum)
		.def("addSpectrum", &Environment::addSpectrum)
		.def("getMaterial", &Environment::getMaterial)
		.def("hasMaterial", &Environment::hasMaterial)
		.def("addMaterial", &Environment::addMaterial)
		.add_property("materialCount", &Environment::materialCount)
		.def("getMesh", &Environment::getMesh)
		.def("hasMesh", &Environment::hasMesh)
		.def("addMesh", &Environment::addMesh)
		.def("addShaderOutput",
			 (void (Environment::*)(const std::string&, const std::shared_ptr<PR::ScalarShaderOutput>&)) & Environment::addShaderOutput)
		.def("addShaderOutput",
			 (void (Environment::*)(const std::string&, const std::shared_ptr<PR::SpectralShaderOutput>&)) & Environment::addShaderOutput)
		.def("addShaderOutput",
			 (void (Environment::*)(const std::string&, const std::shared_ptr<PR::VectorShaderOutput>&)) & Environment::addShaderOutput)
		.def("getScalarShaderOutput", &Environment::getScalarShaderOutput)
		.def("hasScalarShaderOutput", &Environment::hasScalarShaderOutput)
		.def("getSpectralShaderOutput", &Environment::getSpectralShaderOutput)
		.def("hasSpectralShaderOutput", &Environment::hasSpectralShaderOutput)
		.def("getVectorShaderOutput", &Environment::getVectorShaderOutput)
		.def("hasVectorShaderOutput", &Environment::hasVectorShaderOutput)
		.add_property("renderWidth", &Environment::renderWidth, &Environment::setRenderWidth)
		.add_property("renderHeight", &Environment::renderHeight, &Environment::setRenderHeight)
		.def("setCrop", &Environment::setCrop)
		.add_property("cropMinX", &Environment::cropMinX)
		.add_property("cropMaxX", &Environment::cropMaxX)
		.add_property("cropMinY", &Environment::cropMinY)
		.add_property("cropMaxY", &Environment::cropMaxY)
		// TODO
		.def("dumpInformation", &Environment::dumpInformation)
		.def("setup", &Environment::setup)
		.def("save", &Environment::save);

	bpy::class_<SceneLoader, boost::noncopyable>("SceneLoader", bpy::no_init)
		.def("loadFromString", &SceneLoader::loadFromString)
		.staticmethod("loadFromString")
		.def("loadFromFile", &SceneLoader::loadFromFile)
		.staticmethod("loadFromFile");
}
}