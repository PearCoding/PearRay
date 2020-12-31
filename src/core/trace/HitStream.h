#pragma once

#include "HitEntry.h"
#include "shader/ShadingGroupBlock.h"

#include <array>
#include <vector>

namespace PR {

class PR_LIB_CORE HitStream {
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
	void setup(bool sort);
	void reset();
	ShadingGroupBlock getNextGroup();

	size_t getMemoryUsage() const;

private:
	std::vector<uint32> mRayID;
	std::vector<uint32> mEntityID;
	std::vector<uint32> mPrimitiveID;
	std::vector<float> mParameter[3];

	size_t mSize;
	size_t mCurrentPos;
};
} // namespace PR
