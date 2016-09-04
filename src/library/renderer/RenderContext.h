#pragma once

#include "thread/Thread.h"
#include "renderer/RenderStatistics.h"
#include "Random.h"

namespace PR
{
	class Renderer;
	class RenderThread;
	class RenderEntity;
	struct SamplePoint;
	class Ray;
	class Sampler;
	class Spectrum;
	class PR_LIB RenderContext
	{
	public:
		RenderContext(Renderer* renderer, RenderThread* thread, uint32 index);
		~RenderContext();

		void render(uint32 x, uint32 y, uint32 sample, uint32 pass);
		RenderEntity* shoot(const Ray& ray, SamplePoint& collisionPoint, RenderEntity* ignore = nullptr);
		RenderEntity* shootWithEmission(Spectrum& appliedSpec, const Ray& ray, SamplePoint& collisionPoint, RenderEntity* ignore = nullptr);

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

		inline Random& random()
		{
			return mRandom;
		}

		inline Sampler* pixelSampler() const
		{
			return mPixelSampler;
		}

		inline void setPixelSampler(Sampler* sampler)
		{
			mPixelSampler = sampler;
		}

		inline const RenderStatistics& stats() const
		{
			return mStatistics;
		}

		inline RenderStatistics& stats()
		{
			return mStatistics;
		}

	private:
		Renderer* mRenderer;
		RenderThread* mThread;
		uint32 mIndex;
		Random mRandom;
		Sampler* mPixelSampler;
		RenderStatistics mStatistics;
	};
}