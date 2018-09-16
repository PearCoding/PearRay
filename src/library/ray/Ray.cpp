#include "Ray.h"

namespace PR {
Ray::Ray()
	: mPixelIndex(0)
	, mOrigin(0, 0, 0)
	, mDirection(0, 0, 0)
	, mDepth(0)
	, mTime(0)
	, mWavelengthIndex(0)
	, mFlags(0)
	, mWeight(1)
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	, mMaxDirectionIndex(0)
#endif
{
	for(int i = 0; i < _RDT_Count; ++i) {
		mOriginDiff[(RayDiffType)i] = Eigen::Vector3f(0,0,0);
		mDirectionDiff[(RayDiffType)i] = Eigen::Vector3f(0,0,0);
	}
}

Ray::Ray(float weight, uint32 pixelIndex, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
		 uint16 depth, float time, uint8 wavelengthIndex, uint8 flags)
	: mPixelIndex(pixelIndex)
	, mOrigin(pos)
	, mDirection(dir)
	, mDepth(depth)
	, mTime(time)
	, mWavelengthIndex(wavelengthIndex)
	, mFlags(flags)
	, mWeight(weight)
{
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	calcMaxDirectionElement();
#endif
	for(int i = 0; i < _RDT_Count; ++i) {
		mOriginDiff[(RayDiffType)i] = Eigen::Vector3f(0,0,0);
		mDirectionDiff[(RayDiffType)i] = Eigen::Vector3f(0,0,0);
	}
}

Ray::~Ray()
{
}
} // namespace PR
