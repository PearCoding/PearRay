#pragma once

#include "RenderThreadContext.h"
#include "thread/Thread.h"

namespace PR {
class RenderContext;
class RenderTile;
class PR_LIB RenderThread : public Thread {
public:
	RenderThread(RenderContext* renderer, uint32 index);

	inline RenderTile* currentTile() const
	{
		return mTile;
	}

	inline RenderThreadContext& context()
	{
		return mContext;
	}

protected:
	virtual void main();

private:
	RenderContext* mRenderer;
	RenderTile* mTile;
	RenderThreadContext mContext;
};
}
