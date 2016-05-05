#pragma once

#include "thread/Thread.h"
#include "RenderContext.h"

namespace PR
{
	class Renderer;
	class PR_LIB RenderThread : public Thread
	{
	public:
		RenderThread(Renderer* renderer, uint32 index);

		size_t pixelsRendered() const;

	protected:
		virtual void main();

	private:
		Renderer* mRenderer;
		RenderContext mContext;

		size_t mPixelsRendered;
	};
}