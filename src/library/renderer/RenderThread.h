#pragma once

#include "thread/Thread.h"

namespace PR {
class RenderContext;
class RenderTile;
class PR_LIB RenderThread : public Thread {
public:
	RenderThread(RenderContext* renderer);

	inline RenderTile* currentTile() const
	{
		return mTile;
	}

protected:
	virtual void main();

private:
	RenderContext* mRenderer;
	RenderTile* mTile;
};
}
