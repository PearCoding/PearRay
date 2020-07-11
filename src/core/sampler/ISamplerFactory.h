#pragma once

#include "PR_Config.h"

namespace PR {
class ISampler;
class Random;
class PR_LIB_CORE ISamplerFactory {
public:
	virtual ~ISamplerFactory() = default;

	virtual uint32 requestedSampleCount() const = 0;
	virtual std::shared_ptr<ISampler> createInstance(uint32 sample_count, Random& rnd) const = 0;
};
} // namespace PR