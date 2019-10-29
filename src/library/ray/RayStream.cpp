#include "RayStream.h"
#include "container/IndexSort.h"
#include <algorithm>
#include <numeric>

#include <fstream>

namespace PR {
#ifdef PR_COMPRESS_RAY_DIR
constexpr int DIR_C_S = 2;
#else
constexpr int DIR_C_S = 3;
#endif

RayGroup::RayGroup(const RayStream* stream, size_t offset, size_t size, bool coherent)
	: mStream(stream)
	, mOffset(offset)
	, mSize(size)
	, mCoherent(coherent)
{
}

/////////////////////////////////////////////////////////

RayStream::RayStream(size_t raycount)
	: mSize(raycount + raycount % PR_SIMD_BANDWIDTH)
	, mCurrentPos(0)
	, mLastInvPos(0)
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].reserve(mSize);
	for (int i = 0; i < DIR_C_S; ++i)
		mDirection[i].reserve(mSize);

	mPixelIndex.reserve(mSize);
	mDepth.reserve(mSize);
	mTime.reserve(mSize);
	mWavelengthIndex.reserve(mSize);
	mFlags.reserve(mSize);
	mWeight.reserve(mSize);
	mInternalIndex.reserve(mSize);
}

RayStream::~RayStream()
{
}

void RayStream::addRay(const Ray& ray)
{
	PR_ASSERT(!isFull(), "Check before adding!");

	for (int i = 0; i < 3; ++i)
		mOrigin[i].emplace_back(ray.Origin[i]);

#ifdef PR_COMPRESS_RAY_DIR
	octNormal16 n(ray.Direction[0], ray.Direction[1], ray.Direction[2]);
	for (int i = 0; i < 2; ++i)
		mDirection[i].emplace_back(n(i));
#else
	for (int i = 0; i < 3; ++i)
		mDirection[i].emplace_back(ray.Direction[i]);
#endif

	mDepth.emplace_back(ray.Depth);
	mPixelIndex.emplace_back(ray.PixelIndex);
	mTime.emplace_back(to_unorm16(ray.Time));
	mWavelengthIndex.emplace_back(ray.WavelengthIndex);
	mFlags.emplace_back(ray.Flags & ~RF_Invalid);
	mWeight.emplace_back(ray.Weight);
	mInternalIndex.emplace_back(mInternalIndex.size());
}

void RayStream::setRay(size_t id, const Ray& ray)
{
	PR_ASSERT(id < currentSize(), "Check before adding!");

	size_t cid = mInternalIndex[id];

	for (int i = 0; i < 3; ++i)
		mOrigin[i][cid] = ray.Origin[i];

#ifdef PR_COMPRESS_RAY_DIR
	octNormal16 n(ray.Direction[0], ray.Direction[1], ray.Direction[2]);
	for (int i = 0; i < 2; ++i)
		mDirection[i][cid] = n(i);
#else
	for (int i = 0; i < 3; ++i)
		mDirection[i][cid] = ray.Direction[i];
#endif

	mDepth[cid]			  = ray.Depth;
	mPixelIndex[cid]	  = ray.PixelIndex;
	mTime[cid]			  = to_unorm16(ray.Time);
	mWavelengthIndex[cid] = ray.WavelengthIndex;
	mFlags[cid]			  = ray.Flags & ~RF_Invalid;
	mWeight[cid]		  = ray.Weight;
}

void RayStream::sort()
{
	// Extract invalid entries out!
	auto inv_start = std::partition(mInternalIndex.begin(), mInternalIndex.end(),
									[&](size_t ind) {
										return (mFlags[ind] & RF_Invalid) == 0;
									});

	mCurrentPos = 0;
	mLastInvPos = std::distance(mInternalIndex.begin(), inv_start);

	// Check coherence
	// TODO
}

void RayStream::reset()
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].clear();

	for (int i = 0; i < DIR_C_S; ++i)
		mDirection[i].clear();

	mDepth.clear();
	mPixelIndex.clear();
	mTime.clear();
	mWavelengthIndex.clear();
	mFlags.clear();
	mWeight.clear();
	mInternalIndex.clear();

	mCurrentPos = 0;
	mLastInvPos = 0;
}

/* A algorithm grouping rays together for coherent intersections
 * would give this function more meaning.
 */
RayGroup RayStream::getNextGroup()
{
	PR_ASSERT(hasNextGroup(), "Never call when not available");

	RayGroup grp(this, 0, mLastInvPos, false);
	mCurrentPos += grp.size();
	return grp;
}

#ifdef PR_COMPRESS_RAY_DIR
#define COMPRES_MEM 2 * sizeof(snorm16)
#else
#define COMPRES_MEM 3 * sizeof(float)
#endif

size_t RayStream::getMemoryUsage() const
{
	return mSize * (3 * sizeof(float) + COMPRES_MEM + sizeof(uint32) + sizeof(uint16) + sizeof(unorm16) + 2 * sizeof(uint8) + sizeof(float) + sizeof(size_t));
}

Ray RayStream::getRay(size_t id) const
{
	PR_ASSERT(id < currentSize(), "Invalid access!");

	const size_t cid = mInternalIndex[id];

	Ray ray;
	ray.Origin = Vector3f(mOrigin[0][cid],
						  mOrigin[1][cid],
						  mOrigin[2][cid]);

#ifdef PR_COMPRESS_RAY_DIR
	float d0 = from_snorm16(mDirection[0][cid]);
	float d1 = from_snorm16(mDirection[1][cid]);
	float dir[3];
	from_oct(d0, d1, dir[0], dir[1], dir[2]);
	ray.Direction = Vector3f(dir[0],
							 dir[1],
							 dir[2]);
#else
	ray.Direction = Vector3f(mDirection[0][cid],
							 mDirection[1][cid],
							 mDirection[2][cid]);
#endif

	ray.Depth			= mDepth[cid];
	ray.PixelIndex		= mPixelIndex[cid];
	ray.Time			= from_unorm16(mTime[cid]);
	ray.WavelengthIndex = mWavelengthIndex[cid];
	ray.Flags			= mFlags[cid] & ~RF_Invalid;
	ray.Weight			= mWeight[cid];

	ray.normalize();
	return ray;
}

RayPackage RayStream::getRayPackage(size_t id) const
{
	RayPackage ray;
	ray.Origin = Vector3fv(load_from_container_with_indices(mInternalIndex, id, mOrigin[0]),
						   load_from_container_with_indices(mInternalIndex, id, mOrigin[1]),
						   load_from_container_with_indices(mInternalIndex, id, mOrigin[2]));

#ifdef PR_COMPRESS_RAY_DIR
	PR_SIMD_ALIGN float dir[3][PR_SIMD_BANDWIDTH];
	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
		size_t cid = mInternalIndex.at(id + k);
		from_oct(
			from_snorm16(Direction[0][cid]),
			from_snorm16(Direction[1][cid]),
			dir[0][k], dir[1][k], dir[2][k]);
	}

	ray.Direction = Vector3fv(simdpp::load(&dir[0][0]),
							  simdpp::load(&dir[1][0]),
							  simdpp::load(&dir[2][0]));
#else
	ray.Direction = Vector3fv(load_from_container_with_indices(mInternalIndex, id, mDirection[0]),
							  load_from_container_with_indices(mInternalIndex, id, mDirection[1]),
							  load_from_container_with_indices(mInternalIndex, id, mDirection[2]));
#endif

	ray.Depth	  = load_from_container_with_indices(mInternalIndex, id, mDepth);
	ray.PixelIndex = load_from_container_with_indices(mInternalIndex, id, mPixelIndex);

	PR_SIMD_ALIGN float t[PR_SIMD_BANDWIDTH];
	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
		size_t cid = mInternalIndex.at(id + k);
		t[k]	   = from_unorm16(mTime[cid]);
	}
	ray.Time = simdpp::load(&t[0]);

	ray.WavelengthIndex = load_from_container_with_indices(mInternalIndex, id, mWavelengthIndex);
	ray.Flags			= load_from_container_with_indices(mInternalIndex, id, mFlags) & vuint32(~RF_Invalid);
	ray.Weight			= load_from_container_with_indices(mInternalIndex, id, mWeight);

	ray.normalize();

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
