#pragma once

#include "thread/Thread.h"

namespace PR
{
	class Renderer;
	class RenderThread;
	class RenderEntity;
	class FacePoint;
	class Ray;
	class PR_LIB RenderContext
	{
	public:
		RenderContext(Renderer* renderer, RenderThread* thread, uint32 index);

		RenderEntity* shoot(Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore = nullptr);
		RenderEntity* shootWithApply(Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore = nullptr);

		inline Renderer* renderer() const
		{
			return mRenderer;
		}

		inline RenderThread* thread() const
		{
			return mThread;
		}

		inline uint32 threadNumber() const
		{
			return mIndex;
		}

	private:
		Renderer* mRenderer;
		RenderThread* mThread;
		uint32 mIndex;
	};
}