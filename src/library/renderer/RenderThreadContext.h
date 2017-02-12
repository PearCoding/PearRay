#pragma once

#include "thread/Thread.h"
#include "renderer/RenderStatistics.h"
#include "renderer/RenderContext.h"
#include "Random.h"

namespace PR
{
	class RenderThread;
	class RenderEntity;
	struct ShaderClosure;
	class Ray;
	class Sampler;
	class Spectrum;
	class PR_LIB RenderThreadContext
	{
	public:
		RenderThreadContext(RenderContext* renderer, RenderThread* thread, uint32 index);
		~RenderThreadContext();

		inline void render(uint32 x, uint32 y, uint32 sample, uint32 pass)
		{
			mRenderer->render(this, x, y, sample, pass);
		}

		inline RenderEntity* shoot(const Ray& ray, ShaderClosure& sc)
		{
			return mRenderer->shoot(ray, sc, this);
		}

		inline bool shootForDetection(const Ray& ray)
		{
			return mRenderer->shootForDetection(ray, this);
		}

		inline RenderEntity* shootWithEmission(Spectrum& appliedSpec, const Ray& ray, ShaderClosure& sc)
		{
			return mRenderer->shootWithEmission(appliedSpec, ray, sc, this);
		}

		inline RenderContext* renderer() const
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
		RenderContext* mRenderer;
		RenderThread* mThread;
		uint32 mIndex;
		Random mRandom;
		Sampler* mPixelSampler;
		RenderStatistics mStatistics;
	};
}
