#pragma once

#include "PR_Config.h"

#include <functional>
#include <vector>

namespace PR {
class Scene;
class RenderContext;
class PR_LIB_LOADER ServiceRegistry {
	friend class ServiceDistributor;

public:
	ServiceRegistry()  = default;
	~ServiceRegistry() = default;

	using BeforeSceneBuildCallback = std::function<void()>;
	using AfterSceneBuildCallback  = std::function<void(Scene*)>;
	using BeforeRenderCallback	   = std::function<void(RenderContext*)>;
	using AfterRenderCallback	   = std::function<void(RenderContext*)>;

	inline void addBeforeSceneBuildCB(const BeforeSceneBuildCallback& cb) { mBeforeSceneBuildCB.push_back(cb); }
	inline void addAfterSceneBuildCB(const AfterSceneBuildCallback& cb) { mAfterSceneBuildCB.push_back(cb); }
	inline void addBeforeRenderCB(const BeforeRenderCallback& cb) { mBeforeRenderCB.push_back(cb); }
	inline void addAfterRenderCB(const AfterRenderCallback& cb) { mAfterRenderCB.push_back(cb); }

private:
	std::vector<BeforeSceneBuildCallback> mBeforeSceneBuildCB;
	std::vector<AfterSceneBuildCallback> mAfterSceneBuildCB;
	std::vector<BeforeRenderCallback> mBeforeRenderCB;
	std::vector<AfterRenderCallback> mAfterRenderCB;
};
} // namespace PR
