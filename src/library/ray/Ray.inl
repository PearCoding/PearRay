#pragma once

namespace PR {
inline void Ray::setWeight(float f)
{
	mWeight = f;
}

inline float Ray::weight() const
{
	return mWeight;
}

inline void Ray::setOrigin(const Eigen::Vector3f& p)
{
	mOrigin = p;
}

inline Eigen::Vector3f Ray::origin() const
{
	return mOrigin;
}

inline void Ray::setDirection(const Eigen::Vector3f& dir)
{
	mDirection = dir;
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	calcMaxDirectionElement();
#endif
}

inline Eigen::Vector3f Ray::direction() const
{
	return mDirection;
}

inline void Ray::setPixelIndex(uint32 p)
{
	mPixelIndex = p;
}

inline uint32 Ray::pixelIndex() const
{
	return mPixelIndex;
}

inline void Ray::setOriginDiff(RayDiffType type, const Eigen::Vector3f& p)
{
	mOriginDiff[type] = p;
}

inline Eigen::Vector3f Ray::originDiff(RayDiffType type) const
{
	return mOriginDiff[type];
}

inline void Ray::setDirectionDiff(RayDiffType type, const Eigen::Vector3f& dir)
{
	mDirectionDiff[type] = dir;
}

inline Eigen::Vector3f Ray::directionDiff(RayDiffType type) const
{
	return mDirectionDiff[type];
}

inline void Ray::setDepth(uint16 depth)
{
	mDepth = depth;
}

inline uint16 Ray::depth() const
{
	return mDepth;
}

inline float Ray::time() const
{
	return mTime;
}

inline void Ray::setTime(float t)
{
	mTime = t;
}

inline uint8 Ray::wavelengthIndex() const
{
	return mWavelengthIndex;
}

inline void Ray::setWavelengthIndex(uint8 index)
{
	mWavelengthIndex = index;
}

inline void Ray::setFlags(uint8 flags)
{
	mFlags = flags;
}

inline uint8 Ray::flags() const
{
	return mFlags;
}

inline Ray Ray::next(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir) const
{
	Ray tmp = *this;
	tmp.setOrigin(safePosition(pos, dir));
	tmp.setDirection(dir);
	tmp.setDepth(mDepth + 1);
	return tmp;
}

inline Eigen::Vector3f Ray::safePosition(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir)
{
	Eigen::Vector3f off	= dir * RayOffsetEpsilon;
	Eigen::Vector3f posOff = pos + off;

	for (int i = 0; i < 3; ++i) {
		if (off(i) > 0)
			posOff(i) = std::nextafter(posOff(i), std::numeric_limits<float>::max());
		else if (off(i) < 0)
			posOff(i) = std::nextafter(posOff(i), std::numeric_limits<float>::lowest());
	}

	return posOff;
}

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
inline uint32 Ray::maxDirectionIndex() const
{
	return mMaxDirectionIndex;
}

inline void Ray::calcMaxDirectionElement()
{
	mMaxDirectionIndex = 0;
	float maxVal	   = 0;
	for (uint32 i = 0; i < 3; ++i) {
		const float f = std::abs(mDirection(i));
		if (maxVal < f) {
			mMaxDirectionIndex = i;
			maxVal			   = f;
		}
	}
}
#endif
} // namespace PR
