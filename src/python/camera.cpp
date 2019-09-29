#include "camera/ICamera.h"
#include "renderer/RenderContext.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_camera(py::module& m)
{
	py::class_<ICamera, std::shared_ptr<ICamera>>(m, "ICamera");
}
}