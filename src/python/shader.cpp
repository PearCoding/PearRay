#include "material/Material.h"
#include "shader/ShaderOutput.h"

// Implementations
#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ConstVectorOutput.h"
#include "shader/ImageScalarOutput.h"
#include "shader/ImageSpectralOutput.h"
#include "shader/ImageVectorOutput.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

void setup_shader(py::module& m)
{
	py::class_<ScalarShaderOutput, std::shared_ptr<ScalarShaderOutput>>(m, "ScalarShaderOutput");

	py::class_<SpectrumShaderOutput, std::shared_ptr<SpectrumShaderOutput>>(m, "SpectrumShaderOutput");

	py::class_<VectorShaderOutput, std::shared_ptr<VectorShaderOutput>>(m, "VectorShaderOutput");

	// Implementations
	py::class_<ConstScalarShaderOutput, ScalarShaderOutput, std::shared_ptr<ConstScalarShaderOutput>>(m, "ConstScalarShaderOutput")
		.def(py::init<float>());

	py::class_<ConstSpectrumShaderOutput, SpectrumShaderOutput, std::shared_ptr<ConstSpectrumShaderOutput>>(m, "ConstSpectrumShaderOutput")
		.def(py::init<const Spectrum&>());

	py::class_<ConstVectorShaderOutput, VectorShaderOutput, std::shared_ptr<ConstVectorShaderOutput>>(m, "ConstVectorShaderOutput")
		.def(py::init<const Eigen::Vector3f&>());

	// py::class_<ImageScalarOutput, std::shared_ptr<ImageScalarOutput>, py::bases<ScalarShaderOutput >
	//     ("ImageScalarOutput", py::init<float>())
	// ;
	// py::class_<ImageSpectralOutput, std::shared_ptr<ImageSpectralOutput>, py::bases<SpectrumShaderOutput >
	//     ("ImageSpectralOutput", py::init<const PR::Spectrum&>())
	// ;
	// py::class_<ImageVectorOutput, std::shared_ptr<ImageVectorOutput>, py::bases<VectorShaderOutput >
	//     ("ImageVectorOutput", py::init<const Eigen::Vector3f&>())
	// ;
}
} // namespace PRPY