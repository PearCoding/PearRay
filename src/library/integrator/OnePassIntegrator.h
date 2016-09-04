#pragma once

#include "Integrator.h"

namespace PR
{
	class PR_LIB OnePassIntegrator : public Integrator
	{
	public:
		virtual void onStart() override;
		virtual void onNextPass(uint32 i) override;
		virtual void onEnd() override;

		virtual void onThreadStart(RenderContext* context) override;
		virtual void onPrePass(RenderContext* context, uint32 i) override;
		virtual void onPass(RenderTile* tile, RenderContext* context, uint32 i) override;
		virtual void onPostPass(RenderContext* context, uint32 i)  override;
		virtual void onThreadEnd(RenderContext* context) override;

		virtual bool needNextPass(uint32 i) const override;
	};
}