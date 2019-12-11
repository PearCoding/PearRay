#include "sampler/ISampler.h"
#include "Random.h"
#include "sampler/HaltonSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/SobolSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class SamplerWrap : public ISampler {
public:
	inline float generate1D(uint32 index) override
	{
		PYBIND11_OVERLOAD_PURE(float, ISampler, generate1D, index);
	}

	inline Vector2f generate2D(uint32 index) override
	{
		PYBIND11_OVERLOAD_PURE(Vector2f, ISampler, generate2D, index);
	}
};

PR_NO_SANITIZE_ADDRESS
void setup_sampler(py::module& m)
{
	py::class_<ISampler, SamplerWrap>(m, "ISampler")
		.def("generate1D", &ISampler::generate1D)
		.def("generate2D", &ISampler::generate2D);

	py::class_<HaltonSampler, ISampler>(m, "HaltonSampler")
		.def(py::init<uint32, uint32, uint32>(),
			 py::arg("samples"), py::arg("baseX") = 13, py::arg("baseY") = 47);

	py::class_<MultiJitteredSampler, ISampler>(m, "MultiJitteredSampler")
		.def(py::init<Random&, uint32>());

	py::class_<RandomSampler, ISampler>(m, "RandomSampler")
		.def(py::init<Random&>());

	py::class_<StratifiedSampler, ISampler>(m, "StratifiedSampler")
		.def(py::init<Random&, uint32>());

	py::class_<SobolSampler, ISampler>(m, "SobolSampler")
		.def(py::init<Random&, uint32>());

	py::class_<UniformSampler, ISampler>(m, "UniformSampler")
		.def(py::init<uint32>());

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
} // namespace PRPY