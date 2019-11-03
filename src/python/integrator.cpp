#include "integrator/IIntegrator.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_integrator(py::module& m)
{
	py::class_<IIntegrator, std::shared_ptr<IIntegrator>>(m, "IIntegrator");
}
}