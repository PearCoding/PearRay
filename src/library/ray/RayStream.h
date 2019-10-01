#pragma once

#include "math/Compression.h"
#include "math/SIMD.h"
#include "ray/RayPackage.h"

namespace PR {
struct PR_LIB_INLINE RayGroup {
	class RayStream* Stream;
	bool Coherent;
	size_t Size;

	float* Origin[3];
	snorm16* Direction[2];
	uint32* PixelIndex;
	uint16* Depth;
	unorm16* Time;
	uint8* WavelengthIndex;
	uint8* Flags;
	float* Weight;

	Ray getRay(size_t id) const;
	RayPackage getRayPackage(size_t id) const;
};

class PR_LIB RayStream {
public:
	explicit RayStream(size_t size);
	virtual ~RayStream();

	inline bool isFull() const { return currentSize() >= maxSize(); }
	inline bool enoughSpace(size_t requested = 1) const { return currentSize() + requested <= maxSize(); }
	inline bool hasNextGroup() const { return mCurrentPos < currentSize(); }

	inline size_t maxSize() const { return mSize; }
	inline size_t currentSize() const { return mWeight.size(); }

	void add(const Ray& ray);
	Ray getRay(size_t id) const;

	void sort();
	void reset();
	RayGroup getNextGroup();

	size_t getMemoryUsage() const;
	void dump(const std::string& file) const;

private: // Some vectors are not aligned, due to required preprocessing
	/* SoA style */
	simd_vector<float> mOrigin[3];

	// OCD Compressed normals
	std::vector<snorm16> mDirection[2];

	simd_vector<uint32> mPixelIndex;

	// TODO: Ray Differentials
	std::vector<uint16> mDepth;
	std::vector<unorm16> mTime;
	std::vector<uint8> mWavelengthIndex;
	std::vector<uint8> mFlags;

	simd_vector<float> mWeight;

	size_t mSize;
	size_t mCurrentPos;
};
} // namespace PR