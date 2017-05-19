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
				context->render(Eigen::Vector2i(x, y), tile->samplesRendered(), pass);
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

	RenderStatus OnePassIntegrator::status() const
	{
		const uint64 max_samples =
			renderer()->width() * renderer()->height() * renderer()->settings().maxCameraSampleCount();
		
		RenderStatus stat;
		stat.setField("int.max_sample_count", max_samples);
		stat.setField("int.max_pass_count", (uint64)1);

		stat.setPercentage(renderer()->statistics().pixelSampleCount() / (float)max_samples);

		return stat;
	}
}
