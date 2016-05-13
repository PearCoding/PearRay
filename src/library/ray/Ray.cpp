#include "Ray.h"

namespace PR
{
	Ray::Ray() :
		mStartPosition(PM::pm_Set(0,0,0,1)), mDirection(PM::pm_Zero()), mDepth(0), mMaxDepth(0), mFlags(RF_DefaultCollision)
	{
	}

	Ray::Ray(const PM::vec3& pos, const PM::vec3& dir, uint32 depth) :
		mStartPosition(pos), mDirection(dir), mDepth(depth), mMaxDepth(0), mFlags(RF_DefaultCollision)
	{
	}

	Ray::~Ray()
	{
	}
}