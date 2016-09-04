#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	enum PPMGatheringMode
	{
		PGM_Sphere,
		PGM_Dome
	};

	class PR_LIB PPMSettings
	{
	public:
		PPMSettings();

		inline uint32 maxPhotonsPerPass() const { return mMaxPhotonsPerPass; }
		inline void setMaxPhotonsPerPass(uint32 v) { mMaxPhotonsPerPass = v; }

		inline uint32 maxPassCount() const { return mMaxPassCount; }
		inline void setMaxPassCount(uint32 v) { mMaxPassCount = v; }

		inline float maxGatherRadius() const { return mMaxGatherRadius; }
		inline void setMaxGatherRadius(float v) { mMaxGatherRadius = v; }

		inline uint32 maxGatherCount() const { return mMaxGatherCount; }
		inline void setMaxGatherCount(uint32 v) { mMaxGatherCount = v; }

		inline PPMGatheringMode gatheringMode() const { return mGatheringMode; }
		inline void setGatheringMode(PPMGatheringMode v) { mGatheringMode = v; }

		inline float squeezeWeight() const { return mSqueezeWeight; }
		inline void setSqueezeWeight(float v) { mSqueezeWeight = v; }

	private:
		uint32 mMaxPhotonsPerPass;
		uint32 mMaxPassCount;

		float mMaxGatherRadius;
		uint32 mMaxGatherCount;
		PPMGatheringMode mGatheringMode;
		float mSqueezeWeight;
	};
}