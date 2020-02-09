#pragma once

#include "math/Compression.h"
#include "math/SIMD.h"
#include "ray/RayPackage.h"

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
	const size_t mOffset;
	const size_t mSize;
	const bool mCoherent;
};

class PR_LIB RayStream {
public:
	explicit RayStream(size_t size);
	virtual ~RayStream();

	inline bool isFull() const { return currentSize() >= maxSize(); }
	inline bool enoughSpace(size_t requested = 1) const { return currentSize() + requested <= maxSize(); }
	inline bool hasNextGroup() const { return mCurrentPos < currentSize(); }

	inline size_t maxSize() const { return mSize; }
	inline size_t currentSize() const { return mPixelIndex.size(); }

	void addRay(const Ray& ray);
	Ray getRay(size_t id) const;
	RayPackage getRayPackage(size_t id) const;

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
	std::vector<uint16> mIterationDepth;
	std::vector<unorm16> mTime;
	std::vector<uint8> mWavelengthIndex;
	std::vector<uint8> mFlags;

	std::vector<float> mMinT;
	std::vector<float> mMaxT;
	std::vector<float> mWeight[3];

	size_t mSize;
	size_t mCurrentPos;
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
} // namespace PR
