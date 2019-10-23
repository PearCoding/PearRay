#include "RayStream.h"
#include "container/IndexSort.h"
#include <algorithm>
#include <numeric>

#include <fstream>

namespace PR {
/* This implementation has a huge amount of code-repetition, due to multiple arrays...
* --> FIXME: Doing more abstract interfaces would solve this problem, but speed is very important here.
*/

constexpr uint8 RAY_INVALID = 0xFF;

#ifdef PR_COMPRESS_RAY_DIR
constexpr int DIR_C_S = 2;
#else
constexpr int DIR_C_S = 3;
#endif

Ray RayGroup::getRay(size_t id) const
{
	PR_ASSERT(id < Size, "Invalid access!");

	Ray ray;
	ray.Origin = Vector3f(Origin[0][id], Origin[1][id], Origin[2][id]);

#ifdef PR_COMPRESS_RAY_DIR
	float d0 = from_snorm16(Direction[0][id]);
	float d1 = from_snorm16(Direction[1][id]);
	float d[3];
	from_oct(d0, d1, d[0], d[1], d[2]);
	ray.Direction = Vector3f(d[0], d[1], d[2]);
#else
	ray.Direction = Vector3f(Direction[0][id], Direction[1][id], Direction[2][id]);
#endif

	ray.Depth			= Depth[id];
	ray.PixelIndex		= PixelIndex[id];
	ray.SessionIndex	= SessionIndex[id];
	ray.Time			= from_unorm16(Time[id]);
	ray.WavelengthIndex = WavelengthIndex[id];
	ray.Flags			= Flags[id];
	ray.Weight			= Weight[id];

	ray.normalize();
	return ray;
}

RayPackage RayGroup::getRayPackage(size_t id) const
{
	PR_ASSERT(id < Size, "Invalid access!");

	RayPackage ray;
	ray.Origin = Vector3fv(simdpp::load(&Origin[0][id]),
						   simdpp::load(&Origin[1][id]),
						   simdpp::load(&Origin[2][id]));

#ifdef PR_COMPRESS_RAY_DIR
	PR_SIMD_ALIGN float dir[3][PR_SIMD_BANDWIDTH];
	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
		from_oct(
			from_snorm16(Direction[0][id + k]),
			from_snorm16(Direction[1][id + k]),
			dir[0][k], dir[1][k], dir[2][k]);
	}

	ray.Direction = Vector3fv(simdpp::load(&dir[0][0]),
							  simdpp::load(&dir[1][0]),
							  simdpp::load(&dir[2][0]));
#else
	ray.Direction = Vector3fv(simdpp::load(&Direction[0][id]),
							  simdpp::load(&Direction[1][id]),
							  simdpp::load(&Direction[2][id]));
#endif

	load_from_container_linear(ray.Depth, Depth, id);
	load_from_container_linear(ray.PixelIndex, PixelIndex, id);
	load_from_container_linear(ray.SessionIndex, SessionIndex, id);

	PR_SIMD_ALIGN float t[PR_SIMD_BANDWIDTH];
	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k) {
		t[k] = from_unorm16(Time[id + k]);
	}
	ray.Time = simdpp::load(&t[0]);

	load_from_container_linear(ray.WavelengthIndex, WavelengthIndex, id);
	load_from_container_linear(ray.Flags, Flags, id);
	load_from_container_linear(ray.Weight, Weight, id);

	ray.normalize();

	return ray;
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
	mSessionIndex.reserve(mSize);
	mDepth.reserve(mSize);
	mTime.reserve(mSize);
	mWavelengthIndex.reserve(mSize);
	mFlags.reserve(mSize);
	mWeight.reserve(mSize);
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
	mSessionIndex.emplace_back(ray.SessionIndex);
	mTime.emplace_back(to_unorm16(ray.Time));
	mWavelengthIndex.emplace_back(ray.WavelengthIndex);
	mFlags.emplace_back(ray.Flags & ~RF_BackgroundHit);
	mWeight.emplace_back(ray.Weight);
}

void RayStream::setRay(size_t id, const Ray& ray)
{
	PR_ASSERT(id < currentSize(), "Check before adding!");

	for (int i = 0; i < 3; ++i)
		mOrigin[i][id] = ray.Origin[i];

#ifdef PR_COMPRESS_RAY_DIR
	octNormal16 n(ray.Direction[0], ray.Direction[1], ray.Direction[2]);
	for (int i = 0; i < 2; ++i)
		mDirection[i][id] = n(i);
#else
	for (int i = 0; i < 3; ++i)
		mDirection[i][id] = ray.Direction[i];
#endif

	mDepth[id]			 = ray.Depth;
	mPixelIndex[id]		 = ray.PixelIndex;
	mSessionIndex[id]	= ray.SessionIndex;
	mTime[id]			 = to_unorm16(ray.Time);
	mWavelengthIndex[id] = ray.WavelengthIndex;
	mFlags[id]			 = ray.Flags & ~RF_BackgroundHit;
	mWeight[id]			 = ray.Weight;
}
void RayStream::invalidateRay(size_t id)
{

	PR_ASSERT(id < currentSize(), "Check before adding!");
	mFlags[id] = RAY_INVALID;
}

void RayStream::sort()
{
	std::vector<size_t> index(currentSize());
	std::iota(index.begin(), index.end(), 0);

	// Extract invalid entries out!
	auto inv_start = std::partition_point(index.begin(), index.end(), [&](size_t ind) {
		return mFlags[ind] != RAY_INVALID;
	});

	mCurrentPos = 0;
	mLastInvPos = std::distance(index.begin(), inv_start);

	// Check coherence
	// TODO

	// Rearrange arrays
	sortByIndex(
		[&](size_t oldI, size_t newI) {
			std::swap(mDepth[oldI], mDepth[newI]);
			std::swap(mPixelIndex[oldI], mPixelIndex[newI]);
			std::swap(mSessionIndex[oldI], mSessionIndex[newI]);
			std::swap(mTime[oldI], mTime[newI]);
			std::swap(mWavelengthIndex[oldI], mWavelengthIndex[newI]);
			std::swap(mFlags[oldI], mFlags[newI]);
			std::swap(mWeight[oldI], mWeight[newI]);
		},
		index);
}

void RayStream::reset()
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].clear();

	for (int i = 0; i < DIR_C_S; ++i)
		mDirection[i].clear();

	mDepth.clear();
	mPixelIndex.clear();
	mSessionIndex.clear();
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
	grp.Size	 = mLastInvPos;

	for (int i = 0; i < 3; ++i)
		grp.Origin[i] = &mOrigin[i].data()[mCurrentPos];
	for (int i = 0; i < DIR_C_S; ++i)
		grp.Direction[i] = &mDirection[i].data()[mCurrentPos];
	grp.PixelIndex		= &mPixelIndex.data()[mCurrentPos];
	grp.SessionIndex	= &mSessionIndex.data()[mCurrentPos];
	grp.Depth			= &mDepth.data()[mCurrentPos];
	grp.Time			= &mTime.data()[mCurrentPos];
	grp.WavelengthIndex = &mWavelengthIndex.data()[mCurrentPos];
	grp.Flags			= &mFlags.data()[mCurrentPos];
	grp.Weight			= &mWeight.data()[mCurrentPos];

	mCurrentPos += grp.Size;

	return grp;
}

size_t RayStream::getMemoryUsage() const
{
	return mSize * (3 * sizeof(float) + 2 * sizeof(snorm16) + sizeof(uint32) + sizeof(uint16) + sizeof(unorm16) + 2 * sizeof(uint8) + sizeof(float));
}

Ray RayStream::getRay(size_t id) const
{
	PR_ASSERT(id < currentSize(), "Invalid access!");

	Ray ray;
	ray.Origin = Vector3f(mOrigin[0][id],
						  mOrigin[1][id],
						  mOrigin[2][id]);

#ifdef PR_COMPRESS_RAY_DIR
	float d0 = from_snorm16(mDirection[0][id]);
	float d1 = from_snorm16(mDirection[1][id]);
	float dir[3];
	from_oct(d0, d1, dir[0], dir[1], dir[2]);
	ray.Direction = Vector3f(dir[0],
							 dir[1],
							 dir[2]);
#else
	ray.Direction = Vector3f(mDirection[0][id],
							 mDirection[1][id],
							 mDirection[2][id]);
#endif

	ray.Depth			= mDepth[id];
	ray.PixelIndex		= mPixelIndex[id];
	ray.SessionIndex	= mSessionIndex[id];
	ray.Time			= from_unorm16(mTime[id]);
	ray.WavelengthIndex = mWavelengthIndex[id];
	ray.Flags			= mFlags[id];
	ray.Weight			= mWeight[id];

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
