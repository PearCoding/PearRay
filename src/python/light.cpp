#include "light/DistantLight.h"
#include "light/EnvironmentLight.h"
#include "light/IInfiniteLight.h"
#include "material/Material.h"
#include "renderer/RenderContext.h"
#include "shader/ShaderClosure.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class IInfiniteLightWrap : public IInfiniteLight {
public:
	inline Spectrum apply(const Eigen::Vector3f& L) override
	{
		PYBIND11_OVERLOAD_PURE(Spectrum, IInfiniteLight, apply, L);
	}

	inline LightSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override
	{
		PYBIND11_OVERLOAD_PURE(LightSample, IInfiniteLight, sample, point, rnd);
	}

	inline void onFreeze() override
	{
		PYBIND11_OVERLOAD(void, IInfiniteLight, onFreeze);
	}
};

void setup_light(py::module& m)
{
	auto scope = py::class_<IInfiniteLight, IInfiniteLightWrap, std::shared_ptr<IInfiniteLight>>(m, "IInfiniteLight");
	scope.def("apply", &IInfiniteLight::apply)
		.def("sample", &IInfiniteLight::sample)
		.def("onFreeze", &IInfiniteLight::onFreeze)
		.def("freeze", &IInfiniteLight::freeze)
		.def_property_readonly("frozen", &IInfiniteLight::isFrozen);

	py::class_<IInfiniteLight::LightSample>(scope, "LightSample")
		.def_readwrite("PDF_S", &IInfiniteLight::LightSample::PDF_S)
		.def_readwrite("L", &IInfiniteLight::LightSample::L);

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