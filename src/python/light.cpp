#include "light/DistantLight.h"
#include "light/EnvironmentLight.h"
#include "light/IInfiniteLight.h"
#include "material/Material.h"
#include "renderer/RenderContext.h"
#include "shader/ShaderClosure.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_light(py::module& m)
{
	auto scope = py::class_<IInfiniteLight, std::shared_ptr<IInfiniteLight>>(m, "IInfiniteLight");
	scope.def_property_readonly("frozen", &IInfiniteLight::isFrozen);

	py::class_<DistantLight, std::shared_ptr<DistantLight>, IInfiniteLight>(m, "DistantLight")
		.def_property("direction", &DistantLight::direction, &DistantLight::setDirection)
		.def_property("material",
					  &DistantLight::material,
					  &DistantLight::setMaterial);

	py::class_<EnvironmentLight, std::shared_ptr<EnvironmentLight>, IInfiniteLight>(m, "EnvironmentLight")
		.def_property("material",
					  &EnvironmentLight::material,
					  &EnvironmentLight::setMaterial);
}
}