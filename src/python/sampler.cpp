#include "Random.h"
#include "sampler/HaltonQMCSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/Sampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"
#include <boost/python.hpp>

#include "npmath.h"

using namespace PR;
namespace bpy = boost::python;
namespace np  = boost::python::numpy;

namespace PRPY {
class SamplerWrap : public Sampler, public bpy::wrapper<Sampler> {
public:
	inline float generate1D(uint32 index) override
	{
		return this->get_override("generate1D")(index);
	}

	inline Eigen::Vector2f generate2D(uint32 index) override
	{
		return vec2FromPython(generate2D_Py(index));
	}

	inline np::ndarray generate2D_Py(uint32 index)
	{
		return this->get_override("generate2D")(index);
	}

	inline Eigen::Vector3f generate3D(uint32 index) override
	{
		return vec3FromPython(generate3D_Py(index));
	}

	inline np::ndarray generate3D_Py(uint32 index)
	{
		return this->get_override("generate3D")(index);
	}
};

class RandomWrap : public Random, public bpy::wrapper<Random> {
public:
	RandomWrap(uint64 seed)
		: Random(seed)
	{
	}

	np::ndarray get2D_Py()
	{
		return vec2ToPython(get2D());
	}

	np::ndarray get3D_Py()
	{
		return vec3ToPython(get3D());
	}

	np::ndarray get4D_Py()
	{
		return vec4ToPython(get4D());
	}
};

void setup_sampler()
{
	bpy::class_<SamplerWrap, boost::noncopyable>("Sampler")
		.def("generate1D", bpy::pure_virtual(&Sampler::generate1D))
		.def("generate2D", bpy::pure_virtual(&SamplerWrap::generate2D_Py))
		.def("generate3D", bpy::pure_virtual(&SamplerWrap::generate3D_Py));

	bpy::class_<HaltonQMCSampler, bpy::bases<Sampler>, boost::noncopyable>("HaltonQMCSampler", bpy::init<uint32, bpy::optional<bool, uint32, uint32, uint32>>());

	bpy::class_<MultiJitteredSampler, bpy::bases<Sampler>, boost::noncopyable>("MultiJitteredSampler", bpy::init<Random&, uint32>());

	bpy::class_<RandomSampler, bpy::bases<Sampler>, boost::noncopyable>("RandomSampler", bpy::init<Random&>());

	bpy::class_<StratifiedSampler, bpy::bases<Sampler>, boost::noncopyable>("StratifiedSampler", bpy::init<Random&, uint32>());

	bpy::class_<UniformSampler, bpy::bases<Sampler>, boost::noncopyable>("UniformSampler", bpy::init<Random&, uint32>());

	bpy::class_<RandomWrap, boost::noncopyable>("Random", bpy::init<uint64>())
		.def("get32", (uint32_t(Random::*)()) & Random::get32)
		.def("get32", (uint32_t(Random::*)(uint32_t, uint32_t)) & Random::get32)
		.def("get64", (uint64_t(Random::*)()) & Random::get64)
		.def("get64", (uint64_t(Random::*)(uint64_t, uint64_t)) & Random::get64)
		.def("getFloat", &Random::getFloat)
		.def("getDouble", &Random::getDouble)
		.def("get2D", &RandomWrap::get2D_Py)
		.def("get3D", &RandomWrap::get3D_Py)
		.def("get4D", &RandomWrap::get4D_Py);
}
}