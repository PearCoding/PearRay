#include "material/IMaterial.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_material(py::module& m)
{
	py::class_<IMaterial, std::shared_ptr<IMaterial>>(m, "IMaterial")
		.def("dumpInformation", &IMaterial::dumpInformation)
		.def_property_readonly("id", &IMaterial::id)
		.def_property("shadeable", &IMaterial::canBeShaded, &IMaterial::enableShading)
		.def_property("shadow", &IMaterial::allowsShadow, &IMaterial::enableShadow)
		.def_property("selfShadow", &IMaterial::allowsSelfShadow, &IMaterial::enableSelfShadow)
		.def_property("cameraVisible", &IMaterial::isCameraVisible, &IMaterial::enableCameraVisibility);
}
}