#pragma once

namespace PR {
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

inline void Ray::setPixel(const Eigen::Vector2i& p)
{
	mPixel = p;
}

inline Eigen::Vector2i Ray::pixel() const
{
	return mPixel;
}

inline void Ray::setXOrigin(const Eigen::Vector3f& p)
{
	mXOrigin = p;
}

inline Eigen::Vector3f Ray::xorigin() const
{
	return mXOrigin;
}

inline void Ray::setXDirection(const Eigen::Vector3f& dir)
{
	mXDirection = dir;
}

inline Eigen::Vector3f Ray::xdirection() const
{
	return mXDirection;
}

inline void Ray::setYOrigin(const Eigen::Vector3f& p)
{
	mYOrigin = p;
}

inline Eigen::Vector3f Ray::yorigin() const
{
	return mYOrigin;
}

inline void Ray::setYDirection(const Eigen::Vector3f& dir)
{
	mYDirection = dir;
}

inline Eigen::Vector3f Ray::ydirection() const
{
	return mYDirection;
}

inline void Ray::setDepth(uint32 depth)
{
	mDepth = depth;
}

inline uint32 Ray::depth() const
{
	return mDepth;
}

inline SI::Time Ray::time() const
{
	return mTime;
}

inline void Ray::setTime(const SI::Time& t)
{
	mTime = t;
}

inline uint8 Ray::wavelength() const
{
	return mWavelengthIndex;
}

inline void Ray::setWavelength(uint8 wavelength)
{
	PR_ASSERT(wavelength < Spectrum::SAMPLING_COUNT,
			  "Given wavelenght greater than SAMPLING_COUNT");
	mWavelengthIndex = wavelength;
}

inline void Ray::setFlags(uint16 flags)
{
	mFlags = flags;
}

inline uint16 Ray::flags() const
{
	return mFlags;
}

inline Ray Ray::next(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir) const
{
	return safe(mPixel, pos, dir, mDepth + 1, mTime, mWavelengthIndex, mFlags);
}

inline Ray Ray::safe(const Eigen::Vector2i& pixel, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
					 uint32 depth, const SI::Time& time, uint8 wavelength, uint16 flags)
{
	Eigen::Vector3f off	= dir * RayOffsetEpsilon;
	Eigen::Vector3f posOff = pos + off;

	for (int i = 0; i < 3; ++i) {
		if (off(i) > 0)
			posOff(i) = std::nextafter(posOff(i), std::numeric_limits<float>::max());
		else if (off(i) < 0)
			posOff(i) = std::nextafter(posOff(i), std::numeric_limits<float>::lowest());
	}

	return Ray(pixel, posOff, dir, depth, time, wavelength, flags);
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
}
