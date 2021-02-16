#pragma once

#include "PR_Config.h"

namespace PR {
class ISpectralMapper;
class RenderContext;
class PR_LIB_CORE ISpectralMapperFactory {
public:
	virtual ~ISpectralMapperFactory() = default;

	virtual std::shared_ptr<ISpectralMapper> createInstance(RenderContext* ctx) = 0;
};
} // namespace PR
