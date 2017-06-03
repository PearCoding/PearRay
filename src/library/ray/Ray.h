#pragma once

#include "spectral/Spectrum.h"
#include "SIMath.h"
#include <Eigen/Dense>

namespace PR {
enum RayFlags {
	RF_Light = 0x1,
	RF_Debug = 0x2
};

constexpr float RayOffsetEpsilon = 0.000001f;
class PR_LIB Ray {
public:
	Ray();
	Ray(const Eigen::Vector2i& pixel, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
		uint32 depth = 0, const SI::Time& time = 0, uint8 wavelength = 0, uint16 flags = 0);
	virtual ~Ray();

	inline void setOrigin(const Eigen::Vector3f& p);
	inline Eigen::Vector3f origin() const;

	inline void setDirection(const Eigen::Vector3f& p);
	inline Eigen::Vector3f direction() const;

	inline void setPixel(const Eigen::Vector2i& pixel);
	inline Eigen::Vector2i pixel() const;

	inline void setXOrigin(const Eigen::Vector3f& p);
	inline Eigen::Vector3f xorigin() const;

	inline void setXDirection(const Eigen::Vector3f& p);
	inline Eigen::Vector3f xdirection() const;

	inline void setYOrigin(const Eigen::Vector3f& p);
	inline Eigen::Vector3f yorigin() const;

	inline void setYDirection(const Eigen::Vector3f& p);
	inline Eigen::Vector3f ydirection() const;

	inline void setDepth(uint32 depth);
	inline uint32 depth() const;

	inline SI::Time time() const;
	inline void setTime(const SI::Time& t);

	inline uint8 wavelength() const;
	inline void setWavelength(uint8 wavelength);

	inline void setFlags(uint16 flags);
	inline uint16 flags() const;

// Used by Triangle.h
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	inline uint32 maxDirectionIndex() const;
#endif

	inline Ray next(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir) const;
	static inline Ray safe(const Eigen::Vector2i& pixel, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
						   uint32 depth = 0, const SI::Time& time = 0, uint8 wavelength = 0, uint16 flags = 0);

private:
	Eigen::Vector3f mOrigin;
	Eigen::Vector3f mDirection;
	Eigen::Vector2i mPixel;

	// Ray differentials
	Eigen::Vector3f mXOrigin;
	Eigen::Vector3f mXDirection;

	Eigen::Vector3f mYOrigin;
	Eigen::Vector3f mYDirection;

	uint32 mDepth; // Recursion depth!
	SI::Time mTime;
	uint8 mWavelengthIndex;
	uint16 mFlags;

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	inline void calcMaxDirectionElement();
	uint32 mMaxDirectionIndex;
#endif
};
}

#include "Ray.inl"
