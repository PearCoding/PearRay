#pragma once

#include "math/Compression.h"
#include "math/SIMD.h"
#include "ray/RayPackage.h"

#define PR_INVALID_RAY_FLAG (0xFF)

namespace PR {
class RayStream;
class PR_LIB RayGroup {
private:
	friend RayStream;

	RayGroup(const RayStream* stream, size_t offset, size_t size, bool coherent);

public:
	~RayGroup()				  = default;
	RayGroup(const RayGroup&) = default;
	RayGroup(RayGroup&&)	  = default;
	RayGroup& operator=(const RayGroup&) = default;
	RayGroup& operator=(RayGroup&&) = default;

	inline Ray getRay(size_t id) const;
	inline RayPackage getRayPackage(size_t id) const;

	inline size_t offset() const { return mOffset; }
	inline size_t size() const { return mSize; }
	inline bool isCoherent() const { return mCoherent; }
	inline const RayStream* stream() const { return mStream; }

private:
	const RayStream* mStream;
	size_t mOffset;
	size_t mSize;
	bool mCoherent;
};

class PR_LIB RayStream {
public:
	explicit RayStream(size_t size);
	virtual ~RayStream();

	inline bool isFull() const { return currentSize() >= maxSize(); }
	inline bool enoughSpace(size_t requested = 1) const { return currentSize() + requested <= maxSize(); }
	inline bool hasNextGroup() const { return mCurrentPos < mLastInvPos; }

	inline size_t maxSize() const { return mSize; }
	inline size_t currentSize() const { return mWeight.size(); }

	inline size_t linearID(size_t id) const { return mInternalIndex.at(id); }

	void addRay(const Ray& ray);
	void setRay(size_t id, const Ray& ray);
	Ray getRay(size_t id) const;
	RayPackage getRayPackage(size_t id) const;
	inline void invalidateRay(size_t id);

	void sort();
	void reset();
	RayGroup getNextGroup();

	size_t getMemoryUsage() const;
	void dump(const std::string& file) const;

private: // Some vectors are not aligned, due to required preprocessing
	/* SoA style */
	std::vector<float> mOrigin[3];

#ifdef PR_COMPRESS_RAY_DIR
	// OCD Compressed normals
	std::vector<snorm16> mDirection[2];
#else
	std::vector<float> mDirection[3];
#endif

	std::vector<uint32> mPixelIndex;

	// TODO: Ray Differentials
	std::vector<uint16> mDepth;
	std::vector<unorm16> mTime;
	std::vector<uint8> mWavelengthIndex;
	std::vector<uint8> mFlags;

	std::vector<float> mWeight;
	std::vector<size_t> mInternalIndex;

	size_t mSize;
	size_t mCurrentPos;
	size_t mLastInvPos;
};

inline Ray RayGroup::getRay(size_t id) const
{
	PR_ASSERT(id < mSize, "Invalid access!");
	return mStream->getRay(id + mOffset);
}

inline RayPackage RayGroup::getRayPackage(size_t id) const
{
	PR_ASSERT(id < mSize, "Invalid access!");
	return mStream->getRayPackage(id + mOffset);
}

inline void RayStream::invalidateRay(size_t id)
{
	PR_ASSERT(id < currentSize(), "Check before adding!");
	mFlags[linearID(id)] = PR_INVALID_RAY_FLAG;
}
} // namespace PR
