#pragma once

#include "thread/Thread.h"

namespace PR
{
	class Renderer;
	class RenderThread;
	class RenderEntity;
	class FacePoint;
	class Ray;
	class Spectrum;
	class PR_LIB RenderContext
	{
	public:
		RenderContext(Renderer* renderer, RenderThread* thread, uint32 index);
		~RenderContext();

		RenderEntity* shoot(const Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore = nullptr);
		RenderEntity* shootWithApply(Spectrum& appliedSpec, const Ray& ray, FacePoint& collisionPoint, RenderEntity* ignore = nullptr);

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