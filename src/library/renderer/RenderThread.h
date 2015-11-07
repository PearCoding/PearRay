#pragma once

#include "thread/Thread.h"

namespace PR
{
	class Renderer;
	class PR_LIB RenderThread : public Thread
	{
	public:
		RenderThread(Renderer* renderer);

		size_t pixelsRendered() const;

	protected:
		virtual void main();

	private:
		Renderer* mRenderer;

		size_t mPixelsRendered;
	};
}