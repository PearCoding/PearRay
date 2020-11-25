#pragma once

#include "renderer/RenderStatus.h"
#include <vector>

namespace PR {
class RenderContext;
class RenderTileSession;

// For each working thread
class PR_LIB_CORE IIntegratorInstance {
public:
	virtual void onStart() {}
	virtual void onEnd() {}

	virtual void onTile(RenderTileSession& session) = 0;
};

struct PR_LIB_CORE IntegratorConfiguration {
	uint32 PassCount = 1;
};

// Main thread
class PR_LIB_CORE IIntegrator {
public:
	explicit IIntegrator();
	virtual ~IIntegrator();

	virtual void onInit(RenderContext*) {}

	virtual void onStart() {}
	virtual void onEnd() {}

	virtual std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext*, size_t) = 0;
	virtual RenderStatus status() const { return RenderStatus(); }
	virtual IntegratorConfiguration configuration() const { return IntegratorConfiguration(); }
};
} // namespace PR
