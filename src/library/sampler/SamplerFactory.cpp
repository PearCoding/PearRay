#include "SamplerFactory.h"
#include "HaltonSampler.h"
#include "MultiJitteredSampler.h"
#include "RandomSampler.h"
#include "SobolSampler.h"
#include "StratifiedSampler.h"
#include "UniformSampler.h"

namespace PR {
std::shared_ptr<ISampler> SamplerFactory::createSampler(SamplerType type, Random& random, size_t samples)
{
	switch (type) {
	case ST_RANDOM:
		return std::make_shared<RandomSampler>(random);
	case ST_UNIFORM:
		return std::make_shared<UniformSampler>(samples);
	case ST_JITTER:
		return std::make_shared<StratifiedSampler>(random, samples);
	default:
	case ST_MULTI_JITTER:
		return std::make_shared<MultiJitteredSampler>(random, samples);
	case ST_HALTON:
		return std::make_shared<HaltonSampler>(samples);
	case ST_SOBOL:
		return std::make_shared<SobolSampler>(random, samples);
	}
}

static struct {
	const char* Name;
	SamplerType Type;
} _s_types[] = {
	{ "default", ST_MULTI_JITTER },
	{ "standard", ST_MULTI_JITTER },
	{ "random", ST_RANDOM },
	{ "uniform", ST_UNIFORM },
	{ "jitter", ST_JITTER },
	{ "multi_jitter", ST_MULTI_JITTER },
	{ "multi-jitter", ST_MULTI_JITTER },
	{ "halton", ST_HALTON },
	{ "sobol", ST_SOBOL },
	{ nullptr, ST_MULTI_JITTER }
};

std::shared_ptr<ISampler> SamplerFactory::createSampler(const std::string& type, Random& random, size_t samples)
{
	std::string name = type;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	for (int i = 0; _s_types[i].Name; ++i) {
		if (name == _s_types[i].Name)
			return createSampler(_s_types[i].Type, random, samples);
	}

	return createSampler(ST_MULTI_JITTER, random, samples);
}
} // namespace PR