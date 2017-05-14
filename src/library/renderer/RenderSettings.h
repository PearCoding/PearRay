#pragma once

#include "PPMSettings.h"

namespace PR
{
	enum SamplerMode
	{
		SM_Random,
		SM_Uniform,
		SM_Jitter,
		SM_MultiJitter,
		SM_HaltonQMC
	};

	enum DebugMode
	{
		DM_None,
		DM_Depth,
		DM_Normal_Both,
		DM_Normal_Positive,
		DM_Normal_Negative,
		DM_Normal_Spherical,
		DM_Tangent_Both,
		DM_Tangent_Positive,
		DM_Tangent_Negative,
		DM_Tangent_Spherical,
		DM_Binormal_Both,
		DM_Binormal_Positive,
		DM_Binormal_Negative,
		DM_Binormal_Spherical,
		DM_UV,
		DM_PDF,
		DM_Emission,
		DM_Validity
	};

	enum IntegratorMode
	{
		IM_Direct,
		IM_BiDirect,
		IM_PPM// Progressive Photon Mapping
	};

	/* Visual feedback tile mode */
	enum TileMode
	{
		TM_Linear,
		TM_Tile,
		TM_Spiral
	};

	enum TimeMappingMode
	{
		TMM_Center,// [0.5, 0.5]
		TMM_Left,// [-1, 0]
		TMM_Right// [0, 1]
	};

	class PR_LIB RenderSettings
	{
	public:
		RenderSettings();

		inline uint32 seed() const { return mSeed; }
		inline void setSeed(uint64 s) { mSeed = s; }
		
		inline bool isIncremental() const { return mIncremental; }
		inline void setIncremental(bool b) { mIncremental = b; }

		inline DebugMode debugMode() const { return mDebugMode; }
		inline void setDebugMode(DebugMode mode) { mDebugMode = mode; }

		inline IntegratorMode integratorMode() const { return mIntegratorMode; }
		inline void setIntegratorMode(IntegratorMode mode) { mIntegratorMode = mode; }

		// Sampler
		inline SamplerMode aaSampler() const { return mAASampler; }
		inline void setAASampler(SamplerMode mode) { mAASampler = mode; }

		inline uint32 maxAASampleCount() const { return mMaxAASampleCount; }
		inline void setMaxAASampleCount(uint32 v) { mMaxAASampleCount = v; }

		inline SamplerMode lensSampler() const { return mLensSampler; }
		inline void setLensSampler(SamplerMode mode) { mLensSampler = mode; }

		inline uint32 maxLensSampleCount() const { return mMaxLensSampleCount; }
		inline void setMaxLensSampleCount(uint32 v) { mMaxLensSampleCount = v; }

		inline SamplerMode timeSampler() const { return mTimeSampler; }
		inline void setTimeSampler(SamplerMode mode) { mTimeSampler = mode; }

		inline uint32 maxTimeSampleCount() const { return mMaxTimeSampleCount; }
		inline void setMaxTimeSampleCount(uint32 v) { mMaxTimeSampleCount = v; }

		inline TimeMappingMode timeMappingMode() const { return mTimeMappingMode; }
		inline void setTimeMappingMode(TimeMappingMode mode) { mTimeMappingMode = mode; }

		inline float timeScale() const { return mTimeScale; }
		inline void setTimeScale(float s) { mTimeScale = s; }

		inline SamplerMode spectralSampler() const { return mSpectralSampler; }
		inline void setSpectralSampler(SamplerMode mode) { mSpectralSampler = mode; }

		inline uint32 maxSpectralSampleCount() const { return mMaxSpectralSampleCount; }
		inline void setMaxSpectralSampleCount(uint32 v) { mMaxSpectralSampleCount = v; }

		inline uint32 maxCameraSampleCount() const
		{
			return mMaxAASampleCount * mMaxLensSampleCount * mMaxTimeSampleCount * mMaxSpectralSampleCount;
		}

		// Rays
		inline uint32 maxDiffuseBounces() const { return mMaxDiffuseBounces; }
		inline void setMaxDiffuseBounces(uint32 v) { mMaxDiffuseBounces = v; }

		inline uint32 maxRayDepth() const { return mMaxRayDepth; }
		inline void setMaxRayDepth(uint32 v) { mMaxRayDepth = v; }

		// Crop
		inline float cropMaxX() const { return mCropMaxX; }
		inline void setCropMaxX(float v) { mCropMaxX = PM::pm_Clamp(v, mCropMinX, 1.0f); }

		inline float cropMinX() const { return mCropMinX; }
		inline void setCropMinX(float v) { mCropMinX = PM::pm_Clamp(v, 0.0f, mCropMaxX); }

		inline float cropMaxY() const { return mCropMaxY; }
		inline void setCropMaxY(float v) { mCropMaxY = PM::pm_Clamp(v, mCropMinY, 1.0f); }

		inline float cropMinY() const { return mCropMinY; }
		inline void setCropMinY(float v) { mCropMinY = PM::pm_Clamp(v, 0.0f, mCropMaxY); }

		// Direct Lightning
		inline uint32 maxLightSamples() const { return mMaxLightSamples; }
		inline void setMaxLightSamples(uint32 v) { mMaxLightSamples = v; }

		// PPM
		inline const PPMSettings& ppm() const { return mPPM; }
		inline PPMSettings& ppm() { return mPPM; }

		// Display Rendering
		inline TileMode tileMode() const { return mTileMode; }
		inline void setTileMode(TileMode m) { mTileMode = m; }
	private:
		uint64 mSeed;
		bool mIncremental;
		DebugMode mDebugMode;
		IntegratorMode mIntegratorMode;

		// Sampler
		SamplerMode mAASampler;
		uint32 mMaxAASampleCount;
		SamplerMode mLensSampler;
		uint32 mMaxLensSampleCount;
		
		SamplerMode mTimeSampler;
		uint32 mMaxTimeSampleCount;
		TimeMappingMode mTimeMappingMode;
		float mTimeScale;

		SamplerMode mSpectralSampler;
		uint32 mMaxSpectralSampleCount;

		// Rays
		uint32 mMaxDiffuseBounces;
		uint32 mMaxRayDepth;

		//Crop
		float mCropMaxX;
		float mCropMinX;
		float mCropMaxY;
		float mCropMinY;

		// Direct Lightning
		uint32 mMaxLightSamples;

		// PPM
		PPMSettings mPPM;

		// Display Rendering
		TileMode mTileMode;
	};
}
