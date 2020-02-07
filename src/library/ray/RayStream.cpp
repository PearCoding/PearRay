#include "RayStream.h"
#include "Platform.h"
#include "Profiler.h"
#include "container/IndexSort.h"

#include <algorithm>
#include <fstream>
#include <numeric>

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
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].reserve(mSize);
	for (int i = 0; i < DIR_C_S; ++i)
		mDirection[i].reserve(mSize);

	mPixelIndex.reserve(mSize);
	mIterationDepth.reserve(mSize);
	mTime.reserve(mSize);
	mWavelengthIndex.reserve(mSize);
	mFlags.reserve(mSize);
	for (int i = 0; i < 3; ++i)
		mWeight[i].reserve(mSize);
}

RayStream::~RayStream()
{
}

void RayStream::addRay(const Ray& ray)
{
	PR_PROFILE_THIS;

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

	mIterationDepth.emplace_back(ray.IterationDepth);
	mPixelIndex.emplace_back(ray.PixelIndex);
	mTime.emplace_back(to_unorm16(ray.Time));
	mWavelengthIndex.emplace_back(ray.WavelengthIndex);
	mFlags.emplace_back(ray.Flags);
	for (int i = 0; i < 3; ++i)
		mWeight[i].emplace_back(ray.Weight(i));
}

void RayStream::reset()
{
	PR_PROFILE_THIS;

	for (int i = 0; i < 3; ++i)
		mOrigin[i].clear();

	for (int i = 0; i < DIR_C_S; ++i)
		mDirection[i].clear();

	mIterationDepth.clear();
	mPixelIndex.clear();
	mTime.clear();
	mWavelengthIndex.clear();
	mFlags.clear();
	for (int i = 0; i < 3; ++i)
		mWeight[i].clear();

	mCurrentPos = 0;
}

/* A algorithm grouping rays together for coherent intersections
 * would give this function more meaning.
 */
RayGroup RayStream::getNextGroup()
{
	PR_PROFILE_THIS;

	PR_ASSERT(hasNextGroup(), "Never call when not available");

	// TODO: Check coherence
	RayGroup grp(this, 0, currentSize(), false);
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
	return mSize * (3 * sizeof(float) + COMPRES_MEM + sizeof(uint32) + sizeof(uint16) + sizeof(unorm16) + 2 * sizeof(uint8) + sizeof(size_t));
}

Ray RayStream::getRay(size_t id) const
{
	PR_PROFILE_THIS;

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

	ray.IterationDepth  = mIterationDepth[id];
	ray.PixelIndex		= mPixelIndex[id];
	ray.Time			= from_unorm16(mTime[id]);
	ray.WavelengthIndex = mWavelengthIndex[id];
	ray.Flags			= mFlags[id];
	ray.Weight			= ColorTriplet(mWeight[0][id],
							   mWeight[1][id],
							   mWeight[2][id]);

	ray.normalize();

	return ray;
}

RayPackage RayStream::getRayPackage(size_t id) const
{
	PR_PROFILE_THIS;

	RayPackage ray;
	load_from_container_linear(ray.Origin[0], mOrigin[0], id);
	load_from_container_linear(ray.Origin[1], mOrigin[1], id);
	load_from_container_linear(ray.Origin[2], mOrigin[2], id);

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
	load_from_container_linear(ray.Direction[0], mDirection[0], id);
	load_from_container_linear(ray.Direction[1], mDirection[1], id);
	load_from_container_linear(ray.Direction[2], mDirection[2], id);
#endif

	load_from_container_linear(ray.IterationDepth, mIterationDepth, id);
	load_from_container_linear(ray.PixelIndex, mPixelIndex, id);

	PR_SIMD_ALIGN float t[PR_SIMD_BANDWIDTH];
	for (size_t k = 0; k < PR_SIMD_BANDWIDTH; ++k)
		t[k] = from_unorm16(mTime[id + k]);
	ray.Time = simdpp::load(&t[0]);

	load_from_container_linear(ray.WavelengthIndex, mWavelengthIndex, id);
	load_from_container_linear(ray.Flags, mFlags, id);

	load_from_container_linear(ray.Weight[0], mWeight[0], id);
	load_from_container_linear(ray.Weight[1], mWeight[1], id);
	load_from_container_linear(ray.Weight[2], mWeight[2], id);

	ray.normalize();

	return ray;
}

// Utility function
void RayStream::dump(const std::string& file) const
{
	std::ofstream stream;
	stream.open(encodePath(file), std::ios::out | std::ios::binary);
	if (!stream)
		return;

	uint64 size = static_cast<uint64>(currentSize());
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
