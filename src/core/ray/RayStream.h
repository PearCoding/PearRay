#pragma once

#include "math/Compression.h"
#include "memory/AlignedAllocator.h"
#include "ray/Ray.h"

#include <filesystem>
#include <vector>

namespace PR {
class RayStream;
class PR_LIB_CORE RaySpan {
private:
	friend RayStream;

	RaySpan(const RayStream* stream, size_t offset, size_t size, bool coherent);

public:
	~RaySpan()				  = default;
	RaySpan(const RaySpan&) = default;
	RaySpan(RaySpan&&)	  = default;

	inline Ray getRay(size_t id) const;
	inline size_t copyOriginX(size_t offset, size_t size, float* dst) const;
	inline size_t copyOriginY(size_t offset, size_t size, float* dst) const;
	inline size_t copyOriginZ(size_t offset, size_t size, float* dst) const;
	inline size_t copyDirectionX(size_t offset, size_t size, float* dst) const;
	inline size_t copyDirectionY(size_t offset, size_t size, float* dst) const;
	inline size_t copyDirectionZ(size_t offset, size_t size, float* dst) const;
	inline size_t copyMaxT(size_t offset, size_t size, float* dst) const;
	inline size_t copyMinT(size_t offset, size_t size, float* dst) const;

	// The raw variants assume valid range
	inline void copyOriginXRaw(size_t offset, size_t size, float* dst) const;
	inline void copyOriginYRaw(size_t offset, size_t size, float* dst) const;
	inline void copyOriginZRaw(size_t offset, size_t size, float* dst) const;
	inline void copyDirectionXRaw(size_t offset, size_t size, float* dst) const;
	inline void copyDirectionYRaw(size_t offset, size_t size, float* dst) const;
	inline void copyDirectionZRaw(size_t offset, size_t size, float* dst) const;
	inline void copyMaxTRaw(size_t offset, size_t size, float* dst) const;
	inline void copyMinTRaw(size_t offset, size_t size, float* dst) const;

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

class PR_LIB_CORE RayStream {
	friend class RaySpan;

	template <typename T>
	using AlignedVector = std::vector<T, AlignedAllocator<T, 64>>;

public:
	explicit RayStream(size_t size);
	virtual ~RayStream();

	inline bool isEmpty() const { return currentSize() == 0; }
	inline bool isFull() const { return currentSize() >= maxSize(); }
	inline bool enoughSpace(size_t requested = 1) const { return currentSize() + requested <= maxSize(); }
	inline bool hasNextSpan() const { return mCurrentReadPos < currentSize(); }

	inline size_t maxSize() const { return mSize; }
	inline size_t currentSize() const { return mCurrentWritePos; }

	void addRay(const Ray& ray);
	Ray getRay(size_t id) const;

	void reset();
	RaySpan getNextSpan();

	size_t getMemoryUsage() const;
	void dump(const std::filesystem::path& file) const;

private: // Some vectors are not aligned, due to required preprocessing
	/* SoA style */
	AlignedVector<float> mOrigin[3];

#if 0 //def PR_COMPRESS_RAY_DIR
	// OCD Compressed normals
	std::vector<snorm16> mDirection[2];
#else
	AlignedVector<float> mDirection[3];
#endif

	AlignedVector<uint32> mPixelIndex;

	// TODO: Ray Differentials
	AlignedVector<uint16> mIterationDepth;
	std::vector<uint8> mFlags;

	AlignedVector<float> mMinT;
	AlignedVector<float> mMaxT;
	AlignedVector<float> mWavelengthNM[PR_SPECTRAL_BLOB_SIZE];
	AlignedVector<uint32> mGroupID;

	size_t mSize;
	size_t mCurrentReadPos;
	size_t mCurrentWritePos;
};

inline Ray RaySpan::getRay(size_t id) const
{
	PR_ASSERT(id < mSize, "Invalid access!");
	return mStream->getRay(id + mOffset);
}

inline size_t RaySpan::copyOriginX(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyOriginXRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyOriginY(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyOriginYRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyOriginZ(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyOriginZRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyDirectionX(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyDirectionXRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyDirectionY(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyDirectionYRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyDirectionZ(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyDirectionZRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyMaxT(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyMaxTRaw(offset, actualsize, dst);
	return actualsize;
}

inline size_t RaySpan::copyMinT(size_t offset, size_t size, float* dst) const
{
	size_t actualsize = std::min(offset + size, mSize) - offset;
	copyMinTRaw(offset, actualsize, dst);
	return actualsize;
}

//////////////
inline void RaySpan::copyOriginXRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mOrigin[0][offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyOriginYRaw(size_t offset, size_t size, float* dst) const
{

	std::memcpy(dst, &mStream->mOrigin[1][offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyOriginZRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mOrigin[2][offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyDirectionXRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mDirection[0][offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyDirectionYRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mDirection[1][offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyDirectionZRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mDirection[2][offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyMaxTRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mMaxT[offset + mOffset], size * sizeof(float));
}

inline void RaySpan::copyMinTRaw(size_t offset, size_t size, float* dst) const
{
	std::memcpy(dst, &mStream->mMinT[offset + mOffset], size * sizeof(float));
}

} // namespace PR
