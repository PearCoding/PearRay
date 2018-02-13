#include "OnePassIntegrator.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderThread.h"
#include "renderer/RenderTile.h"

namespace PR {
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

void OnePassIntegrator::onPass(const RenderSession& session, uint32 pass)
{
	for (uint32 y = session.tile()->sy(); y < session.tile()->ey(); ++y) {
		for (uint32 x = session.tile()->sx(); x < session.tile()->ex(); ++x) {
			renderer()->render(Eigen::Vector2i(x, y), session.tile()->samplesRendered(), pass, session);
		}
	}
}


bool OnePassIntegrator::needNextPass(uint32 i) const
{
	return i == 0;
}

RenderStatus OnePassIntegrator::status() const
{
	const uint64 max_samples = renderer()->width() * renderer()->height() * renderer()->settings().maxCameraSampleCount();

	RenderStatus stat;
	stat.setField("int.max_sample_count", max_samples);
	stat.setField("int.max_pass_count", (uint64)1);

	stat.setPercentage(renderer()->statistics().pixelSampleCount() / (float)max_samples);

	return stat;
}
}
