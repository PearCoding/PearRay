#include "Ray.h"

namespace PR {
Ray::Ray()
	: mStartPosition(0, 0, 0)
	, mDirection(0, 0, 0)
	, mPixel(0, 0)
	, mDepth(0)
	, mTime(0)
	, mWavelengthIndex(0)
	, mFlags(0)
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	, mMaxDirectionIndex(0)
#endif
{
}

Ray::Ray(const Eigen::Vector2i& pixel, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
		 uint32 depth, const SI::Time& time, uint8 wavelength, uint16 flags)
	: mStartPosition(pos)
	, mDirection(dir)
	, mPixel(pixel)
	, mDepth(depth)
	, mTime(time)
	, mWavelengthIndex(wavelength)
	, mFlags(flags)
{
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	calcMaxDirectionElement();
#endif
}

Ray::~Ray()
{
}
}
