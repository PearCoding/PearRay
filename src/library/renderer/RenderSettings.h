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

	enum DebugMode
	{
		DM_None,
		DM_Depth,
		DM_Normal_Both,
		DM_Normal_Positive,
		DM_Normal_Negative,
		DM_Normal_Spherical,
		DM_UV,
		DM_PDF,
		DM_Roughness,
		DM_Reflectivity,
		DM_Transmission
	};

	class PR_LIB RenderSettings
	{
	public:
		RenderSettings();

		inline DebugMode debugMode() const { return mDebugMode; }
		inline void setDebugMode(DebugMode mode) { mDebugMode = mode; }

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

		inline bool isBiDirect() const { return mUseBiDirect; }
		inline void enableBiDirect(bool v) { mUseBiDirect = v; }

		// Photon Mapping
		inline uint32 maxPhotons() const { return mMaxPhotons; }
		inline void setMaxPhotons(uint32 v) { mMaxPhotons = v; }

		inline float maxPhotonGatherRadius() const { return mMaxPhotonGatherRadius; }
		inline void setMaxPhotonGatherRadius(float v) { mMaxPhotonGatherRadius = v; }

		inline uint32 maxPhotonGatherCount() const { return mMaxPhotonGatherCount; }
		inline void setMaxPhotonGatherCount(uint32 v) { mMaxPhotonGatherCount = v; }

		inline uint32 maxPhotonDiffuseBounces() const { return mMaxPhotonDiffuseBounces; }
		inline void setMaxPhotonDiffuseBounces(uint32 v) { mMaxPhotonDiffuseBounces = v; }
	private:
		SamplerMode mSamplerMode;
		uint32 mXSamplerCount;
		uint32 mYSamplerCount;

		uint32 mMaxRayDepth;

		DebugMode mDebugMode;

		// Direct Lightning
		uint32 mMaxLightSamples;
		bool mUseBiDirect;

		// Photon Mapping
		uint32 mMaxPhotons;
		float mMaxPhotonGatherRadius;
		uint32 mMaxPhotonGatherCount;
		uint32 mMaxPhotonDiffuseBounces;
	};
}