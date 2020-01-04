#pragma once

#include "PR_Config.h"

namespace PR {
class ISampler;
class Random;
class PR_LIB ISamplerFactory {
public:
	virtual ~ISamplerFactory() = default;

	virtual std::shared_ptr<ISampler> createInstance(Random& rnd) const = 0;
};
} // namespace PR