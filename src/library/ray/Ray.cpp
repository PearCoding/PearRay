#include "Ray.h"

namespace PR {
Ray::Ray()
	: mOrigin(0, 0, 0)
	, mDirection(0, 0, 0)
	, mPixel(0, 0)
	, mXOrigin(0, 0, 0)
	, mXDirection(0, 0, 0)
	, mYOrigin(0, 0, 0)
	, mYDirection(0, 0, 0)
	, mDepth(0)
	, mTime(0)
	, mWavelength(0)
	, mSpectralStart(0)
	, mSpectralEnd(0)
	, mFlags(0)
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	, mMaxDirectionIndex(0)
#endif
{
}

Ray::Ray(const Eigen::Vector2i& pixel, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
		 uint32 depth, const SI::Time& time, float wavelength, uint16 flags)
	: mOrigin(pos)
	, mDirection(dir)
	, mPixel(pixel)
	, mXOrigin(0, 0, 0)
	, mXDirection(0, 0, 0)
	, mYOrigin(0, 0, 0)
	, mYDirection(0, 0, 0)
	, mDepth(depth)
	, mTime(time)
	, mWavelength(wavelength)
	, mSpectralStart(0)
	, mSpectralEnd(0)
	, mFlags(flags)
{
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	calcMaxDirectionElement();
#endif
}

Ray::~Ray()
{
}
} // namespace PR
