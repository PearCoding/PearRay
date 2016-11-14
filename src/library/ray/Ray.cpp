#include "Ray.h"

namespace PR
{
	Ray::Ray() :
		mStartPosition(PM::pm_Set(0,0,0,1)), mDirection(PM::pm_Set(0,0,0)),
		mDepth(0), mTime(0), mFlags(0),
		mMaxDepth(0)
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		, mMaxDirectionIndex(0)
#endif
	{
	}

	Ray::Ray(const PM::vec3& pixel, const PM::vec3& pos, const PM::vec3& dir, uint32 depth, float time, uint16 flags, uint32 maxDepth) :
		mStartPosition(pos), mDirection(dir), mPixel(pixel),
		mDepth(depth), mTime(time), mFlags(flags),
		mMaxDepth(maxDepth)
	{
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		calcMaxDirectionElement();
#endif
	}

	Ray::~Ray()
	{
	}
}