#include "RenderSession.h"

namespace PR {
RenderSession::RenderSession(uint32 thread, RenderTile* tile)
	: mThread(thread)
	, mTile(tile)
{
}

RenderSession::~RenderSession()
{
}
}
