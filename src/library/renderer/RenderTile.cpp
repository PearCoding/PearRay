#include "RenderTile.h"

namespace PR
{
	RenderTile::RenderTile(uint32 sx, uint32 sy, uint32 ex, uint32 ey) :
		mWorking(false), mSX(sx), mSY(sy), mEX(ex), mEY(ey), mSamplesRendered(0)
	{
	}

	void RenderTile::inc()
	{
		mSamplesRendered++;
	}
	
	void RenderTile::reset()
	{
		mSamplesRendered = 0;
	}
}