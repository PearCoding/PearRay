#pragma once

#include "Random.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderStatistics.h"
#include "thread/Thread.h"

namespace PR {
class RenderThread;
class RenderEntity;
struct ShaderClosure;
class Ray;
class Sampler;
class Spectrum;
class PR_LIB RenderThreadContext {
public:
	RenderThreadContext(RenderContext* renderer, RenderThread* thread, uint32 index);
	~RenderThreadContext();

	inline void render(const Eigen::Vector2i& pixel, uint32 sample, uint32 pass)
	{
		mRenderer->render(this, pixel, sample, pass);
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

	inline Sampler* aaSampler() const
	{
		return mAASampler;
	}

	inline void setAASampler(Sampler* sampler)
	{
		mAASampler = sampler;
	}

	inline Sampler* lensSampler() const
	{
		return mLensSampler;
	}

	inline void setLensSampler(Sampler* sampler)
	{
		mLensSampler = sampler;
	}

	inline Sampler* timeSampler() const
	{
		return mTimeSampler;
	}

	inline void setTimeSampler(Sampler* sampler)
	{
		mTimeSampler = sampler;
	}

	inline Sampler* spectralSampler() const
	{
		return mSpectralSampler;
	}

	inline void setSpectralSampler(Sampler* sampler)
	{
		mSpectralSampler = sampler;
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
	Sampler* mAASampler;
	Sampler* mLensSampler;
	Sampler* mTimeSampler;
	Sampler* mSpectralSampler;
	RenderStatistics mStatistics;
};
}
