#pragma once

#include "HitEntry.h"
#include "shader/ShadingGroup.h"
#include <array>
#include <vector>

namespace PR {

enum HitFlags {
	HF_SUCCESSFUL = 0x1
};

class PR_LIB HitStream {
public:
	explicit HitStream(size_t size);
	virtual ~HitStream();

	inline bool isFull() const { return currentSize() >= maxSize(); }
	inline bool enoughSpace(size_t requested = 1) const { return currentSize() + requested <= maxSize(); }
	inline bool hasNextGroup() const { return mCurrentPos < currentSize(); }

	inline size_t maxSize() const { return mSize; }
	inline size_t currentSize() const { return mRayID.size(); }
	void add(const HitEntry& entry);

	/* Sort order:
	ENTITY_ID,
	MATERIAL_ID
	*/
	void sort();
	void reset();
	ShadingGroup getNextGroup();

	size_t getMemoryUsage() const;

	inline uint32& rayID(size_t index) { return mRayID[index]; }
	inline uint32 rayID(size_t index) const { return mRayID[index]; }

	inline uint32& materialID(size_t index) { return mMaterialID[index]; }
	inline uint32 materialID(size_t index) const { return mMaterialID[index]; }

	inline uint32& entityID(size_t index) { return mEntityID[index]; }
	inline uint32 entityID(size_t index) const { return mEntityID[index]; }

	inline uint32& primitiveID(size_t index) { return mPrimitiveID[index]; }
	inline uint32 primitiveID(size_t index) const { return mPrimitiveID[index]; }

	inline float& uv(size_t elem, size_t index) { return mUV[elem][index]; }
	inline float uv(size_t elem, size_t index) const { return mUV[elem][index]; }

	inline uint8& flags(size_t index) { return mFlags[index]; }
	inline uint8 flags(size_t index) const { return mFlags[index]; }

private:
	std::vector<uint32> mRayID;
	std::vector<uint32> mMaterialID;
	std::vector<uint32> mEntityID;
	std::vector<uint32> mPrimitiveID;
	std::vector<float> mUV[2];
	std::vector<uint8> mFlags;

	size_t mSize;
	size_t mCurrentPos;
};
} // namespace PR
