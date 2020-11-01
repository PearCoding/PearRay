// IWYU pragma: private, include "ServiceObserver.h"

namespace PR {
inline ServiceObserver::CallbackID ServiceObserver::registerBeforeSceneBuild(const BeforeSceneBuildCallback& cb)
{
	const CallbackID id	  = mCounter++;
	mBeforeSceneBuild[id] = cb;
	return id;
}

inline ServiceObserver::CallbackID ServiceObserver::registerAfterSceneBuild(const AfterSceneBuildCallback& cb)
{
	const CallbackID id	 = mCounter++;
	mAfterSceneBuild[id] = cb;
	return id;
}

inline ServiceObserver::CallbackID ServiceObserver::registerBeforeRender(const BeforeRenderCallback& cb)
{
	const CallbackID id = mCounter++;
	mBeforeRender[id]	= cb;
	return id;
}

inline ServiceObserver::CallbackID ServiceObserver::registerAfterRender(const AfterRenderCallback& cb)
{
	const CallbackID id = mCounter++;
	mAfterRender[id]	= cb;
	return id;
}

inline void ServiceObserver::unregister(CallbackID id)
{
	mBeforeSceneBuild.erase(id);
	mAfterSceneBuild.erase(id);
	mBeforeRender.erase(id);
	mAfterRender.erase(id);
}

inline void ServiceObserver::callBeforeSceneBuild() const
{
	for (const auto& it : mBeforeSceneBuild)
		(it.second)();
}

inline void ServiceObserver::callAfterSceneBuild(Scene* scene) const
{
	for (const auto& it : mAfterSceneBuild)
		(it.second)(scene);
}

inline void ServiceObserver::callBeforeRender(RenderContext* ctx) const
{
	for (const auto& it : mBeforeRender)
		(it.second)(ctx);
}

inline void ServiceObserver::callAfterRender(RenderContext* ctx) const
{
	for (const auto& it : mAfterRender)
		(it.second)(ctx);
}
} // namespace PR
