#pragma once

#include "Integrator.h"

namespace PR {
class PR_LIB OnePassIntegrator : public Integrator {
public:
	explicit OnePassIntegrator(RenderContext* renderer)
		: Integrator(renderer)
	{
	}

	void onStart() override;
	void onNextPass(uint32 i, bool& clean) override;
	void onEnd() override;

	void onThreadStart(RenderThreadContext* context) override;
	void onPrePass(RenderThreadContext* context, uint32 i) override;
	void onPass(RenderTile* tile, RenderThreadContext* context, uint32 i) override;
	void onPostPass(RenderThreadContext* context, uint32 i) override;
	void onThreadEnd(RenderThreadContext* context) override;

	bool needNextPass(uint32 i) const override;

	RenderStatus status() const;
};
}
