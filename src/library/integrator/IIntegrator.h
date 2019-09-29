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

	virtual void init(RenderContext* renderer) = 0;

	virtual void onStart()						   = 0;
	virtual void onNextPass(uint32 i, bool& clean) = 0; // Not the main thread!
	virtual void onEnd()						   = 0;
	virtual bool needNextPass(uint32 i) const	  = 0;

	// Per thread
	virtual void onPass(RenderTileSession& session, uint32 pass) = 0;
	virtual RenderStatus status() const							 = 0;
};
} // namespace PR
