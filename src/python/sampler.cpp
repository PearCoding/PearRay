#include "Random.h"
#include "sampler/HaltonQMCSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/Sampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class SamplerWrap : public Sampler {
public:
	inline float generate1D(uint32 index) override
	{
		PYBIND11_OVERLOAD_PURE(float, Sampler, generate1D, index);
	}

	inline Eigen::Vector2f generate2D(uint32 index) override
	{
		PYBIND11_OVERLOAD_PURE(Eigen::Vector2f, Sampler, generate2D, index);
	}

	inline Eigen::Vector3f generate3D(uint32 index) override
	{
		PYBIND11_OVERLOAD_PURE(Eigen::Vector3f, Sampler, generate3D, index);
	}
};

void setup_sampler(py::module& m)
{
	py::class_<Sampler, SamplerWrap>(m, "Sampler")
		.def("generate1D", &Sampler::generate1D)
		.def("generate2D", &Sampler::generate2D)
		.def("generate3D", &Sampler::generate3D);

	py::class_<HaltonQMCSampler, Sampler>(m, "HaltonQMCSampler")
		.def(py::init<uint32, bool, uint32, uint32, uint32>(),
			 py::arg("samples"), py::arg("adaptive") = false, py::arg("baseX") = 13, py::arg("baseY") = 47, py::arg("baseZ") = 89);

	py::class_<MultiJitteredSampler, Sampler>(m, "MultiJitteredSampler")
		.def(py::init<Random&, uint32>());

	py::class_<RandomSampler, Sampler>(m, "RandomSampler")
		.def(py::init<Random&>());

	py::class_<StratifiedSampler, Sampler>(m, "StratifiedSampler")
		.def(py::init<Random&, uint32>());

	py::class_<UniformSampler, Sampler>(m, "UniformSampler")
		.def(py::init<Random&, uint32>());

	py::class_<Random>(m, "Random")
		.def(py::init<uint64>())
		.def("get32", (uint32(Random::*)()) & Random::get32)
		.def("get32", (uint32(Random::*)(uint32, uint32)) & Random::get32)
		.def("get64", (uint64(Random::*)()) & Random::get64)
		.def("get64", (uint64(Random::*)(uint64, uint64)) & Random::get64)
		.def("getFloat", &Random::getFloat)
		.def("getDouble", &Random::getDouble)
		.def("get2D", &Random::get2D)
		.def("get3D", &Random::get3D)
		.def("get4D", &Random::get4D);
}
}