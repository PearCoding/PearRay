#include "RayStream.h"
#include "Ray.h"

namespace PR {

RayStream::RayStream(size_t raycount)
	: mSize(raycount % PR_SIMD_BANDWIDTH)
	, mCurrentPos(0)
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].reserve(mSize);
	for (int i = 0; i < 2; ++i)
		mDirection[i].reserve(mSize);

	mPixelIndex.reserve(mSize);
	mDepth.reserve(mSize);
	mTime.reserve(mSize);
	mWavelengthIndex.reserve(mSize);
	mFlags.reserve(mSize);
	mWeight.reserve(mSize);
}

RayStream::~RayStream()
{
}

void RayStream::add(const Ray& ray)
{
	PR_ASSERT(!isFull(), "Check before adding!");

	for (int i = 0; i < 3; ++i)
		mOrigin[i].emplace_back(ray.origin()(i));

	octNormal16 n(ray.direction());
	for (int i = 0; i < 2; ++i)
		mDirection[i].emplace_back(n(i));

	mDepth.emplace_back(ray.depth());
	mPixelIndex.emplace_back(ray.pixelIndex());
	mTime.emplace_back(to_unorm16(ray.time()));
	mWavelengthIndex.emplace_back(ray.wavelengthIndex());
	mFlags.emplace_back(ray.flags());
	mWeight.emplace_back(ray.weight());
}

void RayStream::sort()
{
	// TODO
}

void RayStream::reset()
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].clear();

	for (int i = 0; i < 2; ++i)
		mDirection[i].clear();

	mDepth.clear();
	mPixelIndex.clear();
	mTime.clear();
	mWavelengthIndex.clear();
	mFlags.clear();
	mWeight.clear();

	mCurrentPos = 0;
}

/* A algorithm grouping rays together for coherent intersections
 * would give this function more meaning. 
 */
RayGroup RayStream::getNextGroup()
{
	PR_ASSERT(hasNextGroup(), "Never call when not available");

	RayGroup grp;
	grp.Stream   = this;
	grp.Coherent = false; //TODO
	grp.Size	 = mWeight.size();

	grp.Origin[0]		= &mOrigin[0].data()[mCurrentPos];
	grp.Origin[1]		= &mOrigin[1].data()[mCurrentPos];
	grp.Origin[2]		= &mOrigin[2].data()[mCurrentPos];
	grp.Direction[0]	= &mDirection[0].data()[mCurrentPos];
	grp.Direction[1]	= &mDirection[1].data()[mCurrentPos];
	grp.PixelIndex		= &mPixelIndex.data()[mCurrentPos];
	grp.Depth			= &mDepth.data()[mCurrentPos];
	grp.Time			= &mTime.data()[mCurrentPos];
	grp.WavelengthIndex = &mWavelengthIndex.data()[mCurrentPos];
	grp.Flags			= &mFlags.data()[mCurrentPos];
	grp.Weight			= &mWeight.data()[mCurrentPos];

	mCurrentPos = mWeight.size(); //TODO

	return grp;
}

size_t RayStream::getMemoryUsage() const
{
	return mSize * (3 * sizeof(float) + 2 * sizeof(snorm16) + sizeof(uint32) + sizeof(uint16) + sizeof(unorm16) + 2 * sizeof(uint8) + sizeof(float));
}
} // namespace PR
