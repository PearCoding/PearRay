#pragma once

#include "Random.h"
#include "renderer/RenderStatistics.h"

namespace PR {

class RenderSettings;
class Sampler;
class PR_LIB RenderTile {
public:
	RenderTile(uint32 sx, uint32 sy, uint32 ex, uint32 ey,
		const RenderSettings& settings, uint32 index);
	~RenderTile();

	void inc();
	void reset();

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
		return mAASampler;
	}
	
	inline Sampler* lensSampler() const
	{
		return mLensSampler;
	}

	inline Sampler* timeSampler() const
	{
		return mTimeSampler;
	}

	inline Sampler* spectralSampler() const
	{
		return mSpectralSampler;
	}

	inline const RenderStatistics& statistics() const
	{
		return mStatistics;
	}

	inline RenderStatistics& statistics()
	{
		return mStatistics;
	}

private:
	bool mWorking;

	const uint32 mSX;
	const uint32 mSY;
	const uint32 mEX;
	const uint32 mEY;
	const uint32 mWidth;
	const uint32 mHeight;
	const uint32 mIndex;
	const uint32 mMaxSamples;

	uint32 mSamplesRendered;

	Random mRandom;
	Sampler* mAASampler;
	Sampler* mLensSampler;
	Sampler* mTimeSampler;
	Sampler* mSpectralSampler;
	RenderStatistics mStatistics;
};
}
