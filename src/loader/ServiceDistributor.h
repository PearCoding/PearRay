#pragma once

#include "ServiceRegistry.h"

namespace PR {
class PR_LIB_LOADER ServiceDistributor {
public:
	inline static void distributeBeforeSceneBuildCB(ServiceRegistry* reg)
	{
		for (const auto& cb : reg->mBeforeSceneBuildCB)
			cb();
	}

	inline static void distributeAfterSceneBuildCB(Scene* scene, ServiceRegistry* reg)
	{
		for (const auto& cb : reg->mAfterSceneBuildCB)
			cb(scene);
	}

	inline static void distributeBeforeRenderCB(RenderContext* ctx, ServiceRegistry* reg)
	{
		for (const auto& cb : reg->mBeforeRenderCB)
			cb(ctx);
	}

	inline static void distributeAfterRenderCB(RenderContext* ctx, ServiceRegistry* reg)
	{
		for (const auto& cb : reg->mAfterRenderCB)
			cb(ctx);
	}
};
} // namespace PR
