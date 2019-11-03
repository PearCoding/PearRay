#pragma once

#include "renderer/RenderStatus.h"
#include <vector>

namespace PR {
class RenderContext;
class RenderTileSession;
class PR_LIB IIntegrator {
public:
	explicit IIntegrator();
	virtual ~IIntegrator();

	// Main thread
	virtual void onInit(RenderContext*) {}

	virtual void onStart() {}
	virtual void onEnd() {}

	virtual bool needNextPass(uint32 i) const { return i == 0; }

	// For each working thread
	virtual void onThreadStart(RenderContext*, size_t) {}
	virtual void onThreadEnd(RenderContext*, size_t) {}

	virtual void onNextPass(uint32, bool&) {}

	virtual void onPass(RenderTileSession& session, uint32 pass) = 0;
	virtual RenderStatus status() const							 = 0;
};
} // namespace PR
