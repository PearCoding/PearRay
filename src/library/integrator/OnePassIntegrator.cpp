#include "OnePassIntegrator.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderThreadContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderThread.h"

namespace PR
{
	void OnePassIntegrator::onStart()
	{
	}

	void OnePassIntegrator::onNextPass(uint32, bool& clean)
	{
		clean = false;
	}

	void OnePassIntegrator::onEnd()
	{
	}

	void OnePassIntegrator::onThreadStart(RenderThreadContext*)
	{
	}

	void OnePassIntegrator::onPrePass(RenderThreadContext*, uint32)
	{
	}

	void OnePassIntegrator::onPass(RenderTile* tile, RenderThreadContext* context, uint32 pass)
	{
		for (uint32 y = tile->sy(); y < tile->ey() && !context->thread()->shouldStop(); ++y)
		{
			for (uint32 x = tile->sx(); x < tile->ex() && !context->thread()->shouldStop(); ++x)
			{
				context->render(x, y, tile->samplesRendered(), pass);
			}
		}
	}

	void OnePassIntegrator::onPostPass(RenderThreadContext*, uint32)
	{
	}

	void OnePassIntegrator::onThreadEnd(RenderThreadContext*)
	{
	}

	bool OnePassIntegrator::needNextPass(uint32 i) const
	{
		return i == 0;
	}

	uint64 OnePassIntegrator::maxSamples(const RenderContext* renderer) const
	{
		return renderer->width() * renderer->height() * renderer->settings().maxPixelSampleCount();
	}

	uint64 OnePassIntegrator::maxPasses(const RenderContext* renderer) const
	{
		return 1;
	}
}
