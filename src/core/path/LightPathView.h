#pragma once

#include "LightPathToken.h"

namespace PR {
/// Lightweight path representation based on packed stream of data
class PR_LIB_CORE LightPathView {
public:
	inline LightPathView(const uint32* data)
		: mData(data + 1) // Skip size entry
		, mSize(data[0])
	{
	}

	inline ~LightPathView() = default;

	inline LightPathToken token(int index) const
	{
		PR_ASSERT(index < (int)mSize, "Expected valid token index");
		return LightPathToken::fromPacked(mData[index]);
	}

	inline size_t containerSize() const { return mSize; }
	inline size_t currentSize() const { return mSize; }

private:
	const uint32* mData;
	const uint32 mSize;
};
} // namespace PR
