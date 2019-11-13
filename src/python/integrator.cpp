#include "integrator/IIntegrator.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_integrator(py::module& m)
{
	py::class_<IIntegrator, std::shared_ptr<IIntegrator>>(m, "IIntegrator");
}
}