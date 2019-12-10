#pragma once

#include "OutputBufferData.h"

namespace PR {
class PR_LIB OutputBuffer {
public:
	explicit OutputBuffer(size_t width, size_t height, size_t specChannels);
	~OutputBuffer();

	inline OutputBufferData& data() { return mData; }
	inline const OutputBufferData& data() const { return mData; }

	inline void clear() { mData.clear(); }

	void pushFragment(uint32 pixelIndex, const ShadingPoint& pt, const LightPath& path);
	void pushFeedbackFragment(uint32 pixelIndex, uint32 feedback);

private:
	OutputBufferData mData;
};
} // namespace PR
