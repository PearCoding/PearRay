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
	HitEntry get(size_t index) const;

	/* Sort order:
	ENTITY_ID,
	MATERIAL_ID
	*/
	void sort();
	void reset();
	ShadingGroup getNextGroup();

	size_t getMemoryUsage() const;

private:
	std::vector<uint32> mRayID;
	std::vector<uint32> mMaterialID;
	std::vector<uint32> mEntityID;
	std::vector<uint32> mPrimitiveID;
	std::vector<float> mParameter[3];
	std::vector<uint8> mFlags;

	size_t mSize;
	size_t mCurrentPos;
};
} // namespace PR
