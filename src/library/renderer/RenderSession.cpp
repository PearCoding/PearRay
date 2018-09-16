#include "RenderSession.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "scene/Scene.h"

namespace PR {
RenderSession::RenderSession(uint32 thread, RenderTile* tile,
							 RayStream* rayStream, HitStream* hitStream)
	: mThread(thread)
	, mTile(tile)
	, mRayStream(rayStream)
	, mHitStream(hitStream)
{
}

RenderSession::~RenderSession()
{
}

IEntity* RenderSession::getEntity(uint32 id) const
{
	return mTile->context().scene()->entities()[id].get();
}
} // namespace PR
