#pragma once

#include "ShadingGroup.h"
#include <vector>
#include <array>

namespace PR {

class PR_LIB HitStream {
public:
	explicit HitStream(size_t size);
	virtual ~HitStream();

	inline bool hasNextGroup() const { return mCurrentPos < mEnd; }

	inline void setEnd(size_t s) { mEnd = s; }

	/* Sort order:
	ENTITY_ID,
	MATERIAL_ID
	*/
	void sort();
	void reset();
	ShadingGroup getNextGroup();

	size_t getMemoryUsage() const;

	uint32& rayID(size_t index) { return mRayID[index]; }
	uint32 rayID(size_t index) const { return mRayID[index]; }

	uint32& materialID(size_t index) { return mMaterialID[index]; }
	uint32 materialID(size_t index) const { return mMaterialID[index]; }

	uint32& entityID(size_t index) { return mEntityID[index]; }
	uint32 entityID(size_t index) const { return mEntityID[index]; }

	uint32& primitiveID(size_t index) { return mPrimitiveID[index]; }
	uint32 primitiveID(size_t index) const { return mPrimitiveID[index]; }

	float& uv(size_t elem, size_t index) { return mUV[elem][index]; }
	float uv(size_t elem, size_t index) const { return mUV[elem][index]; }

	uint8& flags(size_t index) { return mFlags[index]; }
	uint8 flags(size_t index) const { return mFlags[index]; }
private:
	std::vector<uint32> mRayID;
	std::vector<uint32> mMaterialID;
	std::vector<uint32> mEntityID;
	std::vector<uint32> mPrimitiveID;
	std::vector<float> mUV[2];
	std::vector<uint8> mFlags;

	size_t mSize;
	size_t mEnd;
	size_t mCurrentPos;
};
}
