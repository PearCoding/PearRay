#include "Ray.h"

namespace PR
{
	Ray::Ray(const PM::vec3& pos, const PM::vec3& dir, uint32 depth) :
		mStartPosition(pos), mDirection(dir), mTarget(PM::pm_Zero()), mDepth(depth), mMaxDepth(0), mFlags(RF_DefaultCollision)
	{
	}

	Ray::~Ray()
	{
	}
}