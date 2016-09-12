#pragma once

#include "thread/Thread.h"
#include "renderer/RenderStatistics.h"
#include "renderer/Renderer.h"
#include "Random.h"

namespace PR
{
	class Renderer;
	class RenderThread;
	class RenderEntity;
	struct ShaderClosure;
	class Ray;
	class Sampler;
	class Spectrum;
	class PR_LIB RenderContext
	{
	public:
		RenderContext(Renderer* renderer, RenderThread* thread, uint32 index);
		~RenderContext();

		inline void render(uint32 x, uint32 y, uint32 sample, uint32 pass)
		{
			mRenderer->render(this, x, y, sample, pass);
		}
		
		inline RenderEntity* shoot(const Ray& ray, ShaderClosure& sc, RenderEntity* ignore = nullptr)
		{
			return mRenderer->shoot(ray, sc, this, ignore);
		}

		inline RenderEntity* shootForDetection(const Ray& ray, RenderEntity* ignore = nullptr)
		{
			return mRenderer->shootForDetection(ray, this, ignore);
		}

		inline RenderEntity* shootWithEmission(Spectrum& appliedSpec, const Ray& ray, ShaderClosure& sc, RenderEntity* ignore = nullptr)
		{
			return mRenderer->shootWithEmission(appliedSpec, ray, sc, this, ignore);
		}

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