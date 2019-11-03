#include "material/IMaterial.h"
#include "shader/ShadingSocket.h"

// Implementations
#include "shader/ConstShadingSocket.h"
#include "shader/ImageShadingSocket.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

void setup_shader(py::module& m)
{
	py::class_<FloatScalarShadingSocket, std::shared_ptr<FloatScalarShadingSocket>>(m, "FloatScalarShadingSocket");

	py::class_<FloatSpectralShadingSocket, std::shared_ptr<FloatSpectralShadingSocket>>(m, "FloatSpectralShadingSocket");

	py::class_<FloatVectorShadingSocket, std::shared_ptr<FloatVectorShadingSocket>>(m, "FloatVectorShadingSocket");

	// Implementations
	py::class_<ConstScalarShadingSocket, FloatScalarShadingSocket, std::shared_ptr<ConstScalarShadingSocket>>(m, "ConstScalarShadingSocket")
		.def(py::init<float>());

	py::class_<ConstSpectralShadingSocket, FloatSpectralShadingSocket, std::shared_ptr<ConstSpectralShadingSocket>>(m, "ConstSpectralShadingSocket")
		.def(py::init<const Spectrum&>());

	py::class_<ConstVectorShadingSocket, FloatVectorShadingSocket, std::shared_ptr<ConstVectorShadingSocket>>(m, "ConstVectorShadingSocket")
		.def(py::init<const Vector3f&>());

	// py::class_<ImageScalarOutput, std::shared_ptr<ImageScalarOutput>, py::bases<ScalarShadingSocket >
	//     ("ImageScalarOutput", py::init<float>())
	// ;
	// py::class_<ImageSpectralOutput, std::shared_ptr<ImageSpectralOutput>, py::bases<SpectralShadingSocket >
	//     ("ImageSpectralOutput", py::init<const PR::Spectrum&>())
	// ;
	// py::class_<ImageVectorOutput, std::shared_ptr<ImageVectorOutput>, py::bases<VectorShadingSocket >
	//     ("ImageVectorOutput", py::init<const Vector3f&>())
	// ;
}
} // namespace PRPY