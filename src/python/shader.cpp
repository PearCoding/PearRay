#include "material/Material.h"
#include "shader/FacePoint.h"
#include "shader/ShaderClosure.h"
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

class ScalarShaderOutputWrap : public ScalarShaderOutput {
public:
	float eval(const ShaderClosure& point) override
	{
		PYBIND11_OVERLOAD_PURE(float, ScalarShaderOutput, eval, point);
	}
};

class SpectrumShaderOutputWrap : public SpectrumShaderOutput {
public:
	Spectrum eval(const ShaderClosure& point) override
	{
		PYBIND11_OVERLOAD_PURE(Spectrum, SpectrumShaderOutput, eval, point);
	}
};

class VectorShaderOutputWrap : public VectorShaderOutput {
public:
	Eigen::Vector3f eval(const ShaderClosure& point) override
	{
		PYBIND11_OVERLOAD_PURE(Eigen::Vector3f, VectorShaderOutput, eval, point);
	}
};

void setup_shader(py::module& m)
{
	py::class_<FacePoint>(m, "FacePoint")
		.def_readwrite("P", &FacePoint::P)
		.def_readwrite("dPdU", &FacePoint::dPdU)
		.def_readwrite("dPdV", &FacePoint::dPdV)
		.def_readwrite("dPdW", &FacePoint::dPdW)
		.def_readwrite("dPdT", &FacePoint::dPdT)
		.def_readwrite("Ng", &FacePoint::Ng)
		.def_readwrite("Nx", &FacePoint::Nx)
		.def_readwrite("Ny", &FacePoint::Ny)
		.def_readwrite("UVW", &FacePoint::UVW)
		.def_readwrite("dUVWdX", &FacePoint::dUVWdX)
		.def_readwrite("dUVWdY", &FacePoint::dUVWdY)
		.def_readwrite("dUVWdZ", &FacePoint::dUVWdZ)
		.def_readwrite("Material", &FacePoint::Material);

	py::class_<ShaderClosure>(m, "ShaderClosure")
		.def_readwrite("P", &ShaderClosure::P)
		.def_readwrite("dPdU", &ShaderClosure::dPdU)
		.def_readwrite("dPdV", &ShaderClosure::dPdV)
		.def_readwrite("dPdW", &ShaderClosure::dPdW)
		.def_readwrite("dPdT", &ShaderClosure::dPdT)
		.def_readwrite("V", &ShaderClosure::V)
		.def_readwrite("dVdX", &ShaderClosure::dVdX)
		.def_readwrite("dVdY", &ShaderClosure::dVdX)
		.def_readwrite("N", &ShaderClosure::N)
		.def_readwrite("Ng", &ShaderClosure::Ng)
		.def_readwrite("Nx", &ShaderClosure::Nx)
		.def_readwrite("Ny", &ShaderClosure::Ny)
		.def_readwrite("UVW", &ShaderClosure::UVW)
		.def_readwrite("dUVWdX", &ShaderClosure::dUVWdX)
		.def_readwrite("dUVWdY", &ShaderClosure::dUVWdY)
		.def_readwrite("dUVWdZ", &ShaderClosure::dUVWdZ)
		.def_readwrite("T", &ShaderClosure::T)
		.def_readwrite("WavelengthIndex", &ShaderClosure::WavelengthIndex)
		.def_readwrite("Depth2", &ShaderClosure::Depth2)
		.def_readwrite("NgdotV", &ShaderClosure::NgdotV)
		.def_readwrite("NdotV", &ShaderClosure::NdotV)
		.def_readwrite("Flags", &ShaderClosure::Flags)
		.def_readwrite("EntityID", &ShaderClosure::EntityID)
		.def_readwrite("Material", &ShaderClosure::Material);

	py::enum_<ShaderClosureFlags>(m, "ShaderClosureFlags")
		.value("INSIDE", SCF_Inside);

	py::class_<ScalarShaderOutput, ScalarShaderOutputWrap, std::shared_ptr<ScalarShaderOutput>>(m, "ScalarShaderOutput")
		.def("eval", &ScalarShaderOutput::eval);

	py::class_<SpectrumShaderOutput, SpectrumShaderOutputWrap, std::shared_ptr<SpectrumShaderOutput>>(m, "SpectrumShaderOutput")
		.def("eval", &SpectrumShaderOutput::eval);

	py::class_<VectorShaderOutput, VectorShaderOutputWrap, std::shared_ptr<VectorShaderOutput>>(m, "VectorShaderOutput")
		.def("eval", &VectorShaderOutput::eval);

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
}