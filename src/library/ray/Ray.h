#pragma once

#include "spectral/Spectrum.h"
#include <Eigen/Dense>

namespace PR {
enum RayFlags {
	RF_Light = 0x1,
	RF_Debug = 0x2
};

enum RayDiffType {
	RDT_PixelX = 0,
	RDT_PixelY,
	RDT_Time,
	RDT_Wavelength,
	_RDT_Count
};

constexpr float RayOffsetEpsilon = 0.000001f;
class PR_LIB Ray {
public:
	Ray();
	Ray(float weight,
		uint32 pixelIndex, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
		uint16 depth = 0, float time = 0, uint8 wavelengthIndex = 0, uint8 flags = 0);
	virtual ~Ray();

	inline void setWeight(float f);
	inline float weight() const;

	inline void setOrigin(const Eigen::Vector3f& p);
	inline Eigen::Vector3f origin() const;

	inline void setDirection(const Eigen::Vector3f& p);
	inline Eigen::Vector3f direction() const;

	inline void setPixelIndex(uint32 index);
	inline uint32 pixelIndex() const;

	inline void setOriginDiff(RayDiffType type, const Eigen::Vector3f& p);
	inline Eigen::Vector3f originDiff(RayDiffType type) const;

	inline void setDirectionDiff(RayDiffType type, const Eigen::Vector3f& p);
	inline Eigen::Vector3f directionDiff(RayDiffType type) const;

	inline void setDepth(uint16 depth);
	inline uint16 depth() const;

	inline float time() const;
	inline void setTime(float t);

	inline uint8 wavelengthIndex() const;
	inline void setWavelengthIndex(uint8 index);

	inline void setFlags(uint8 flags);
	inline uint8 flags() const;

// Used by Triangle.h
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	inline uint32 maxDirectionIndex() const;
#endif

	inline Ray next(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir) const;
	static inline Eigen::Vector3f safePosition(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir);

private:
	uint32 mPixelIndex;
	Eigen::Vector3f mOrigin;
	Eigen::Vector3f mDirection;

	// Ray differentials
	Eigen::Vector3f mOriginDiff[_RDT_Count];
	Eigen::Vector3f mDirectionDiff[_RDT_Count];

	uint16 mDepth; // Recursion depth!
	float mTime;

	uint8 mWavelengthIndex;
	uint8 mFlags;

	float mWeight;

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	inline void calcMaxDirectionElement();
	uint32 mMaxDirectionIndex;
#endif
};
}

#include "Ray.inl"
