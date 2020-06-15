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
	: mSize(raycount)
	, mCurrentReadPos(0)
	, mCurrentWritePos(0)
{
	for (int i = 0; i < 3; ++i)
		mOrigin[i].resize(mSize);
	for (int i = 0; i < DIR_C_S; ++i)
		mDirection[i].resize(mSize);

	mPixelIndex.resize(mSize);
	mIterationDepth.resize(mSize);
	mTime.resize(mSize);
	mMinT.resize(mSize);
	mMaxT.resize(mSize);
	mFlags.resize(mSize);
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
		mWeight[i].resize(mSize);
		mWavelengthNM[i].resize(mSize);
	}
}

RayStream::~RayStream()
{
}

void RayStream::addRay(const Ray& ray)
{
	PR_PROFILE_THIS;

	//PR_ASSERT(!isFull(), "Check before adding!");
	// TODO: Add dynamic sizing

	for (int i = 0; i < 3; ++i)
		mOrigin[i][mCurrentWritePos] = ray.Origin[i];

#ifdef PR_COMPRESS_RAY_DIR
	octNormal16 n(ray.Direction[0], ray.Direction[1], ray.Direction[2]);
	for (int i = 0; i < 2; ++i)
		mDirection[i][mCurrentWritePos] = n(i);
#else
	for (int i = 0; i < 3; ++i)
		mDirection[i][mCurrentWritePos] = ray.Direction[i];
#endif

	mIterationDepth[mCurrentWritePos] = ray.IterationDepth;
	mPixelIndex[mCurrentWritePos] = ray.PixelIndex;
	mTime[mCurrentWritePos] = to_unorm16(ray.Time);
	mMinT[mCurrentWritePos] = ray.MinT;
	mMaxT[mCurrentWritePos] = ray.MaxT;
	mFlags[mCurrentWritePos] = ray.Flags;
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		mWeight[i][mCurrentWritePos] = ray.Weight(i);

	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		mWavelengthNM[i][mCurrentWritePos] = ray.WavelengthNM(i);

	++mCurrentWritePos;
}

void RayStream::reset()
{
	mCurrentWritePos = 0;
	mCurrentReadPos	 = 0;
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
	mCurrentReadPos += grp.size();
	return grp;
}

#ifdef PR_COMPRESS_RAY_DIR
#define COMPRES_MEM 2 * sizeof(snorm16)
#else
#define COMPRES_MEM 3 * sizeof(float)
#endif

size_t RayStream::getMemoryUsage() const
{
	return mSize * (3 * sizeof(float) + COMPRES_MEM + sizeof(uint32) + sizeof(uint16) + sizeof(unorm16) + sizeof(uint8) + sizeof(size_t) + 2 * sizeof(float) + 2 * PR_SPECTRAL_BLOB_SIZE * sizeof(float));
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

	ray.IterationDepth = mIterationDepth[id];
	ray.PixelIndex	   = mPixelIndex[id];
	ray.Time		   = from_unorm16(mTime[id]);
	ray.Flags		   = mFlags[id];
	ray.MinT		   = mMinT[id];
	ray.MaxT		   = mMaxT[id];
	for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k)
		ray.Weight[k] = mWeight[k][id];
	for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k)
		ray.WavelengthNM[k] = mWavelengthNM[k][id];

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
