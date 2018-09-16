#pragma once

#include "Random.h"
#include "ray/RayPackage.h"
#include "renderer/RenderStatistics.h"

namespace PR {

class RenderContext;
class Sampler;
class PR_LIB RenderTile {
public:
	RenderTile(uint32 sx, uint32 sy, uint32 ex, uint32 ey,
			   const RenderContext& context, uint32 index);
	~RenderTile();

	void inc();
	void reset();

	RayPackage constructCameraRay(const vuint32& px, const vuint32& py, uint32 sample);

	inline bool isWorking() const
	{
		return mWorking;
	}

	inline void setWorking(bool b)
	{
		mWorking = b;
	}

	inline uint32 sx() const
	{
		return mSX;
	}

	inline uint32 sy() const
	{
		return mSY;
	}

	inline uint32 ex() const
	{
		return mEX;
	}

	inline uint32 ey() const
	{
		return mEY;
	}

	inline uint32 width() const
	{
		return mWidth;
	}

	inline uint32 height() const
	{
		return mHeight;
	}

	inline uint32 index() const
	{
		return mIndex;
	}

	inline bool isFinished() const
	{
		return mSamplesRendered >= mMaxSamples;
	}

	inline uint32 samplesRendered() const
	{
		return mSamplesRendered;
	}

	inline Random& random()
	{
		return mRandom;
	}

	inline Sampler* aaSampler() const
	{
		return mAASampler.get();
	}

	inline Sampler* lensSampler() const
	{
		return mLensSampler.get();
	}

	inline Sampler* timeSampler() const
	{
		return mTimeSampler.get();
	}

	inline Sampler* spectralSampler() const
	{
		return mSpectralSampler.get();
	}

	inline const RenderStatistics& statistics() const
	{
		return mStatistics;
	}

	inline RenderStatistics& statistics()
	{
		return mStatistics;
	}

	inline const RenderContext& context() const
	{
		return mContext;
	}

private:
	bool mWorking;

	const uint32 mSX;
	const uint32 mSY;
	const uint32 mEX;
	const uint32 mEY;
	const uint32 mWidth;
	const uint32 mHeight;
	const uint32 mFullWidth;
	const uint32 mFullHeight;
	const uint32 mIndex;
	const uint32 mMaxSamples;

	uint32 mSamplesRendered;

	Random mRandom;
	std::unique_ptr<Sampler> mAASampler;
	std::unique_ptr<Sampler> mLensSampler;
	std::unique_ptr<Sampler> mTimeSampler;
	std::unique_ptr<Sampler> mSpectralSampler;

	const uint32 mAASampleCount;
	const uint32 mLensSampleCount;
	const uint32 mTimeSampleCount;
	const uint32 mSpectralSampleCount;

	// t = t'*alpha + beta
	float mTimeAlpha;
	float mTimeBeta;

	RenderStatistics mStatistics;

	const RenderContext& mContext;
};
} // namespace PR
