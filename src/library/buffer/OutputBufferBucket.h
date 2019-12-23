#pragma once

#include "OutputBufferData.h"
#include "spectral/ColorTriplet.h"

namespace PR {
class IFilter;
class PR_LIB OutputBufferBucket {
	friend class OutputBuffer;
public:
	explicit OutputBufferBucket(const std::shared_ptr<IFilter>& filter,
								size_t width, size_t height, size_t specChannels);
	~OutputBufferBucket();

	inline const OutputBufferData& data() const { return mData; }

	inline void clear(bool force = false) { mData.clear(force); }

	void pushSpectralFragment(uint32 x, uint32 y, const ColorTriplet& spec,
							  uint32 wavelengthIndex, bool isMono, const LightPath& path);
	void pushSPFragment(uint32 x, uint32 y, const ShadingPoint& pt, const LightPath& path);
	void pushFeedbackFragment(uint32 x, uint32 y, uint32 feedback);

	inline size_t originalWidth() const { return mOriginalWidth; }
	inline size_t originalHeight() const { return mOriginalHeight; }
	inline size_t extendedWidth() const { return mExtendedWidth; }
	inline size_t extendedHeight() const { return mExtendedHeight; }

protected:
	void cache();
	inline OutputBufferData& data() { return mData; }

private:
	const std::shared_ptr<IFilter> mFilter;
	const size_t mOriginalWidth;
	const size_t mOriginalHeight;
	const size_t mExtendedWidth;
	const size_t mExtendedHeight;

	OutputBufferData mData;
	bool mHasNonSpecLPE;
};
} // namespace PR
