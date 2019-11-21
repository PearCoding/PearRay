#include "infinitelight/IInfiniteLight.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_infinitelight(py::module& m)
{
	auto scope = py::class_<IInfiniteLight, std::shared_ptr<IInfiniteLight>>(m, "IInfiniteLight");
}
}