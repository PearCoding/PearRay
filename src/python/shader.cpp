#include "material/IMaterial.h"
#include "shader/INode.h"

// Implementations
#include "shader/ConstNode.h"
#include "shader/ImageNode.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_shader(py::module& m)
{
	py::class_<FloatScalarNode, std::shared_ptr<FloatScalarNode>>(m, "FloatScalarNode");

	py::class_<FloatSpectralNode, std::shared_ptr<FloatSpectralNode>>(m, "FloatSpectralNode");

	py::class_<FloatVectorNode, std::shared_ptr<FloatVectorNode>>(m, "FloatVectorNode");

	// Implementations
	py::class_<ConstScalarNode, FloatScalarNode, std::shared_ptr<ConstScalarNode>>(m, "ConstScalarNode")
		.def(py::init<float>());

	py::class_<ConstSpectralNode, FloatSpectralNode, std::shared_ptr<ConstSpectralNode>>(m, "ConstSpectralNode")
		.def(py::init<const ParametricBlob&>());

	py::class_<ConstVectorNode, FloatVectorNode, std::shared_ptr<ConstVectorNode>>(m, "ConstVectorNode")
		.def(py::init<const Vector3f&>());

	// py::class_<ImageScalarOutput, std::shared_ptr<ImageScalarOutput>, py::bases<ScalarNode >
	//     ("ImageScalarOutput", py::init<float>())
	// ;
	// py::class_<ImageSpectralOutput, std::shared_ptr<ImageSpectralOutput>, py::bases<SpectralNode >
	//     ("ImageSpectralOutput", py::init<const PR::Spectrum&>())
	// ;
	// py::class_<ImageVectorOutput, std::shared_ptr<ImageVectorOutput>, py::bases<VectorNode >
	//     ("ImageVectorOutput", py::init<const Vector3f&>())
	// ;
}
} // namespace PRPY