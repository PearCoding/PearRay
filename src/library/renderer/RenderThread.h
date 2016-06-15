#pragma once

#include "thread/Thread.h"
#include "RenderContext.h"

namespace PR
{
	class Renderer;
	class RenderTile;
	class PR_LIB RenderThread : public Thread
	{
	public:
		RenderThread(Renderer* renderer, uint32 index);

		size_t samplesRendered() const;

		inline RenderTile* currentTile() const
		{
			return mTile;
		}

	protected:
		virtual void main();

	private:
		Renderer* mRenderer;
		RenderTile* mTile;
		RenderContext mContext;

		size_t mSamplesRendered;
	};
}