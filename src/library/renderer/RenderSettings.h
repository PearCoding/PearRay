#pragma once

#include "Config.h"
#include "PearMath.h"

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

	enum PhotonGatheringMode
	{
		PGM_Sphere,
		PGM_Dome
	};

	enum IntegratorMode
	{
		IM_Direct,
		IM_BiDirect,
		IM_PPM// Progressive Photon Mapping [TODO]
	};

	class PR_LIB RenderSettings
	{
	public:
		RenderSettings();

		inline bool isIncremental() const { return mIncremental; }
		inline void setIncremental(bool b) { mIncremental = b; }

		inline DebugMode debugMode() const { return mDebugMode; }
		inline void setDebugMode(DebugMode mode) { mDebugMode = mode; }

		inline IntegratorMode integratorMode() const { return mIntegratorMode; }
		inline void setIntegratorMode(IntegratorMode mode) { mIntegratorMode = mode; }

		inline SamplerMode pixelSampler() const { return mPixelSampler; }
		inline void setPixelSampler(SamplerMode mode) { mPixelSampler = mode; }

		inline uint32 maxPixelSampleCount() const { return mMaxPixelSampleCount; }
		inline void setMaxPixelSampleCount(uint32 v) { mMaxPixelSampleCount = v; }

		inline uint32 maxDiffuseBounces() const { return mMaxDiffuseBounces; }
		inline void setMaxDiffuseBounces(uint32 v) { mMaxDiffuseBounces = v; }

		inline uint32 maxRayDepth() const { return mMaxRayDepth; }
		inline void setMaxRayDepth(uint32 v) { mMaxRayDepth = v; }

		// Crop
		inline float cropMaxX() const { return mCropMaxX; }
		inline void setCropMaxX(float v) { mCropMaxX = PM::pm_ClampT(v, mCropMinX, 1.0f); }

		inline float cropMinX() const { return mCropMinX; }
		inline void setCropMinX(float v) { mCropMinX = PM::pm_ClampT(v, 0.0f, mCropMaxX); }

		inline float cropMaxY() const { return mCropMaxY; }
		inline void setCropMaxY(float v) { mCropMaxY = PM::pm_ClampT(v, mCropMinY, 1.0f); }

		inline float cropMinY() const { return mCropMinY; }
		inline void setCropMinY(float v) { mCropMinY = PM::pm_ClampT(v, 0.0f, mCropMaxY); }

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

		inline uint32 maxPhotonDiffuseBounces() const { return mMaxPhotonDiffuseBounces; }
		inline void setMaxPhotonDiffuseBounces(uint32 v) { mMaxPhotonDiffuseBounces = v; }

		inline uint32 minPhotonSpecularBounces() const { return mMinPhotonSpecularBounces; }
		inline void setMinPhotonSpecularBounces(uint32 v) { mMinPhotonSpecularBounces = v; }

		inline PhotonGatheringMode photonGatheringMode() const { return mPhotonGatheringMode; }
		inline void setPhotonGatheringMode(PhotonGatheringMode v) { mPhotonGatheringMode = v; }

		inline float photonSqueezeWeight() const { return mPhotonSqueezeWeight; }
		inline void setPhotonSqueezeWeight(float v) { mPhotonSqueezeWeight = v; }
	private:
		bool mIncremental;
		SamplerMode mPixelSampler;
		uint32 mMaxPixelSampleCount;

		uint32 mMaxDiffuseBounces;
		uint32 mMaxRayDepth;

		DebugMode mDebugMode;
		IntegratorMode mIntegratorMode;

		//Crop
		float mCropMinX;
		float mCropMaxX;
		float mCropMinY;
		float mCropMaxY;

		// Direct Lightning
		uint32 mMaxLightSamples;

		// Photon Mapping
		uint32 mMaxPhotons;
		float mMaxPhotonGatherRadius;
		uint32 mMaxPhotonGatherCount;
		uint32 mMaxPhotonDiffuseBounces;
		uint32 mMinPhotonSpecularBounces;
		PhotonGatheringMode mPhotonGatheringMode;
		float mPhotonSqueezeWeight;
	};
}