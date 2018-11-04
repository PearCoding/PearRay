#include "RenderTileSession.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "scene/Scene.h"

namespace PR {
RenderTileSession::RenderTileSession(uint32 thread, RenderTile* tile,
							 RayStream* rayStream, HitStream* hitStream)
	: mThread(thread)
	, mTile(tile)
	, mRayStream(rayStream)
	, mHitStream(hitStream)
{
}

RenderTileSession::~RenderTileSession()
{
}

IEntity* RenderTileSession::getEntity(uint32 id) const
{
	return mTile->context().scene()->entities()[id].get();
}
} // namespace PR
