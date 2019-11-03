#include "infinitelight/IInfiniteLight.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_infinitelight(py::module& m)
{
	auto scope = py::class_<IInfiniteLight, std::shared_ptr<IInfiniteLight>>(m, "IInfiniteLight");
	scope.def_property_readonly("frozen", &IInfiniteLight::isFrozen);
}
}