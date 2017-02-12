#pragma once

#include "Integrator.h"

namespace PR
{
	class PR_LIB OnePassIntegrator : public Integrator
	{
	public:
		virtual void onStart() override;
		virtual void onNextPass(uint32 i, bool& clean) override;
		virtual void onEnd() override;

		virtual void onThreadStart(RenderThreadContext* context) override;
		virtual void onPrePass(RenderThreadContext* context, uint32 i) override;
		virtual void onPass(RenderTile* tile, RenderThreadContext* context, uint32 i) override;
		virtual void onPostPass(RenderThreadContext* context, uint32 i)  override;
		virtual void onThreadEnd(RenderThreadContext* context) override;

		virtual bool needNextPass(uint32 i) const override;

		virtual uint64 maxSamples(const RenderContext* renderer) const override;
		virtual uint64 maxPasses(const RenderContext* renderer) const override;
	};
}
