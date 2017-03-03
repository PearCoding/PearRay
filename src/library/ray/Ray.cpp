#include "Ray.h"

namespace PR
{
	Ray::Ray() :
		mStartPosition(PM::pm_Set(0,0,0)), mDirection(PM::pm_Set(0,0,0)),
		mPixelX(0), mPixelY(0),
		mDepth(0), mTime(0), mWavelengthIndex(0), mFlags(0)
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		, mMaxDirectionIndex(0)
#endif
	{
	}

	Ray::Ray(uint32 px, uint32 py, const PM::vec3& pos, const PM::vec3& dir, uint32 depth,
		float time, uint8 wavelength, uint16 flags) :
		mStartPosition(pos), mDirection(dir),
		mPixelX(px), mPixelY(py),
		mDepth(depth), mTime(time), mWavelengthIndex(wavelength), mFlags(flags)
	{
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		calcMaxDirectionElement();
#endif
	}

	Ray::~Ray()
	{
	}
}
