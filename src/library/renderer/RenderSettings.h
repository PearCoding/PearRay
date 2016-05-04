#pragma once

#include "Config.h"

namespace PR
{
	enum SamplerMode
	{
		SM_None,
		SM_Random,
		SM_Uniform,
		SM_Jitter
	};

	class PR_LIB RenderSettings
	{
	public:
		RenderSettings();

		inline SamplerMode samplerMode() const { return mSamplerMode; }
		inline void setSamplerMode(SamplerMode mode) { mSamplerMode = mode; }

		inline uint32 xSamplerCount() const { return mXSamplerCount; }
		inline void setXSamplerCount(uint32 v) { mXSamplerCount = v; }

		inline uint32 ySamplerCount() const { return mYSamplerCount; }
		inline void setYSamplerCount(uint32 v) { mYSamplerCount = v; }

		inline uint32 maxRayDepth() const { return mMaxRayDepth; }
		inline void setMaxRayDepth(uint32 v) { mMaxRayDepth = v; }

		// Direct Lightning
		inline uint32 maxLightSamples() const { return mMaxLightSamples; }
		inline void setMaxLightSamples(uint32 v) { mMaxLightSamples = v; }

		// Photon Mapping
		inline uint32 maxPhotons() const { return mMaxPhotons; }
		inline void setMaxPhotons(uint32 v) { mMaxPhotons = v; }

		inline float maxPhotonGatherRadius() const { return mMaxPhotonGatherRadius; }
		inline void setMaxPhotonGatherRadius(float v) { mMaxPhotonGatherRadius = v; }

		inline uint32 maxPhotonGatherCount() const { return mMaxPhotonGatherCount; }
		inline void setMaxPhotonGatherCount(uint32 v) { mMaxPhotonGatherCount = v; }
	private:
		SamplerMode mSamplerMode;
		uint32 mXSamplerCount;
		uint32 mYSamplerCount;

		uint32 mMaxRayDepth;

		// Direct Lightning
		uint32 mMaxLightSamples;

		// Photon Mapping
		uint32 mMaxPhotons;
		float mMaxPhotonGatherRadius;
		uint32 mMaxPhotonGatherCount;
	};
}