#pragma once

#include "PR_Config.h"

namespace PR {
enum SamplerType {
	ST_RANDOM = 0,
	ST_UNIFORM,
	ST_JITTER,
	ST_MULTI_JITTER,
	ST_HALTON,
	ST_SOBOL
};

class ISampler;
class Random;
class PR_LIB SamplerFactory {
public:
	static std::shared_ptr<ISampler> createSampler(SamplerType type, Random& random, size_t count);
	static std::shared_ptr<ISampler> createSampler(const std::string& type, Random& random, size_t count);
};
} // namespace PR