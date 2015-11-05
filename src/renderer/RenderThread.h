#pragma once

#include "thread/Thread.h"

namespace PR
{
	class Renderer;
	class RenderThread : public Thread
	{
	public:
		RenderThread(size_t sx, size_t sy, size_t ex, size_t ey, Renderer* renderer);

	protected:
		virtual void main();

	private:
		size_t mStartX;
		size_t mStartY;
		size_t mEndX;
		size_t mEndY;
		Renderer* mRenderer;
	};
}