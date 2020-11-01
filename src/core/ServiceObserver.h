#pragma once

#include "PR_Config.h"

#include <functional>

namespace PR {
class Scene;
class RenderContext;

class PR_LIB_CORE ServiceObserver {
public:
	using BeforeSceneBuildCallback = std::function<void()>;
	using AfterSceneBuildCallback  = std::function<void(Scene*)>;
	using BeforeRenderCallback	   = std::function<void(RenderContext*)>;
	using AfterRenderCallback	   = std::function<void(RenderContext*)>;

	using CallbackID = size_t;

	inline CallbackID registerBeforeSceneBuild(const BeforeSceneBuildCallback& cb);
	inline CallbackID registerAfterSceneBuild(const AfterSceneBuildCallback& cb);
	inline CallbackID registerBeforeRender(const BeforeRenderCallback& cb);
	inline CallbackID registerAfterRender(const AfterRenderCallback& cb);

	inline void unregister(CallbackID id);

	inline void callBeforeSceneBuild() const;
	inline void callAfterSceneBuild(Scene* scene) const;
	inline void callBeforeRender(RenderContext* ctx) const;
	inline void callAfterRender(RenderContext* ctx) const;

private:
	CallbackID mCounter = 0;
	std::unordered_map<CallbackID, BeforeSceneBuildCallback> mBeforeSceneBuild;
	std::unordered_map<CallbackID, AfterSceneBuildCallback> mAfterSceneBuild;
	std::unordered_map<CallbackID, BeforeRenderCallback> mBeforeRender;
	std::unordered_map<CallbackID, AfterRenderCallback> mAfterRender;
};
} // namespace PR

#include "ServiceObserver.inl"