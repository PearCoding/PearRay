#include "RayStream.h"

#include <fstream>

namespace PR {

Ray RayGroup::getRay(size_t id) const
{
	PR_ASSERT(id < Size, "Invalid access!");

	Ray ray;
	for (int i = 0; i < 3; ++i)
		ray.Origin[i] = Origin[i][id];

	float d0 = from_snorm16(Direction[0][id]);
	float d1 = from_snorm16(Direction[1][id]);
	from_oct(d0, d1, ray.Direction[0], ray.Direction[1], ray.Direction[2]);

	ray.Depth			= Depth[id];
	ray.PixelIndex		= PixelIndex[id];
	ray.Time			= from_unorm16(Time[id]);
	ray.WavelengthIndex = WavelengthIndex[id];
	ray.Flags			= Flags[id];
	ray.Weight			= Weight[id];

	ray.setupInverse();
	return ray;
}

RayPackage RayGroup::getRayPackage(size_t id) const
{
	PR_ASSERT(id < Size, "Invalid access!");

	RayPackage ray;
	PR_SIMD_ALIGN float dir[3][PR_SIMD_BANDWIDTH];

	for (int j = 0; j < 3; ++j)
		ray.Origin[j] = simdpp::load(&Origin[j][id]);

	// Decompress
	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
		from_oct(
			from_snorm16(Direction[0][id + k]),
			from_snorm16(Direction[1][id + k]),
			dir[0][k], dir[1][k], dir[2][k]);
	}

	for (int j = 0; j < 3; ++j)
		ray.Direction[j] = simdpp::load(&dir[j][0]);

	load_from_container_linear(ray.Depth, Depth, id);
	load_from_container_linear(ray.PixelIndex, PixelIndex, id);

	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
		dir[0][k] = from_unorm16(Time[id + k]);
	}
	ray.Time = simdpp::load(&dir[0][id]);

	load_from_container_linear(ray.WavelengthIndex, WavelengthIndex, id);
	load_from_container_linear(ray.Flags, Flags, id);
	load_from_container_linear(ray.Weight, Weight, id);

	ray.setupInverse();

	return ray;
}

RayStream::RayStream(size_t raycount)
	: mSize(raycount + raycount % PR_SIMD_BANDWIDTH)
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
		mOrigin[i].emplace_back(ray.Origin[i]);

	octNormal16 n(ray.Direction[0], ray.Direction[1], ray.Direction[2]);
	for (int i = 0; i < 2; ++i)
		mDirection[i].emplace_back(n(i));

	mDepth.emplace_back(ray.Depth);
	mPixelIndex.emplace_back(ray.PixelIndex);
	mTime.emplace_back(to_unorm16(ray.Time));
	mWavelengthIndex.emplace_back(ray.WavelengthIndex);
	mFlags.emplace_back(ray.Flags & ~RF_BackgroundHit);
	mWeight.emplace_back(ray.Weight);
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

Ray RayStream::getRay(size_t id) const
{
	// FIXME: What happens if we sort?

	PR_ASSERT(id < currentSize(), "Invalid access!");

	Ray ray;
	for (int i = 0; i < 3; ++i)
		ray.Origin[i] = mOrigin[i][id];

	float d0 = from_snorm16(mDirection[0][id]);
	float d1 = from_snorm16(mDirection[1][id]);
	from_oct(d0, d1, ray.Direction[0], ray.Direction[1], ray.Direction[2]);

	ray.Depth			= mDepth[id];
	ray.PixelIndex		= mPixelIndex[id];
	ray.Time			= from_unorm16(mTime[id]);
	ray.WavelengthIndex = mWavelengthIndex[id];
	ray.Flags			= mFlags[id];
	ray.Weight			= mWeight[id];

	ray.setupInverse();
	return ray;
}

// Utility function
void RayStream::dump(const std::string& file) const
{
	std::ofstream stream;
	stream.open(file, std::ios::out | std::ios::binary);
	if (!stream)
		return;

	uint64 size = currentSize();
	stream.write((char*)&size, sizeof(size));

	for (size_t i = 0; i < size; ++i) {
		Ray ray = getRay(i);
		stream.write((char*)&ray.Origin[0], sizeof(float));
		stream.write((char*)&ray.Origin[1], sizeof(float));
		stream.write((char*)&ray.Origin[2], sizeof(float));
		stream.write((char*)&ray.Direction[0], sizeof(float));
		stream.write((char*)&ray.Direction[1], sizeof(float));
		stream.write((char*)&ray.Direction[2], sizeof(float));
	}

	stream.close();
}
} // namespace PR
