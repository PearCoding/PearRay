#include "Ray.h"

namespace PR
{
	Ray::Ray(const PM::vec3& pos, const PM::vec3& dir, size_t depth) :
		mStartPosition(pos), mDirection(dir), mDepth(depth), mMaxDepth(0), mFlags(RF_DefaultCollision)
	{
	}

	Ray::~Ray()
	{
	}
}