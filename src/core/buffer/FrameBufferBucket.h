#pragma once

#include "FrameBufferContainer.h"
#include "filter/FilterCache.h"
#include "spectral/SpectralBlob.h"

#include <array>

namespace PR {
struct OutputSpectralEntry;
struct OutputShadingPointEntry;
struct OutputFeedbackEntry;

class IFilter;
class PR_LIB_CORE FrameBufferBucket {
	friend class FrameBufferSystem;

public:
	explicit FrameBufferBucket(const std::shared_ptr<IFilter>& filter,
							   const Size2i& size, Size1i specChannels, bool monotonic);
	~FrameBufferBucket();

	inline const FrameBufferContainer& data() const { return mData; }

	inline void clear(bool force = false) { mData.clear(force); }

	void commitSpectrals(const OutputSpectralEntry* entries, size_t entrycount);
	void commitShadingPoints(const OutputShadingPointEntry* entries, size_t entrycount);
	void commitFeedbacks(const OutputFeedbackEntry* entries, size_t entrycount);

	inline const Size2i& originalSize() const { return mOriginalSize; }
	inline const Size2i& viewSize() const { return mViewSize; }
	inline const Size2i& extendedSize() const { return mExtendedSize; }
	inline const Size2i& extendedViewSize() const { return mExtendedViewSize; }

	void shrinkView(const Size2i& newView);

protected:
	void cache();
	inline FrameBufferContainer& data() { return mData; }

private:
	void commitSpectralsXYZ(const OutputSpectralEntry* entries, size_t entrycount);
	void commitSpectralsMono(const OutputSpectralEntry* entries, size_t entrycount);

	const FilterCache mFilter;
	const Size2i mOriginalSize;
	const Size2i mExtendedSize;
	const bool mMonotonic;
	Size2i mViewSize;
	Size2i mExtendedViewSize;

	FrameBufferContainer mData;
	bool mHasNonSpecLPE;
	std::array<std::vector<float>, 3> mSpectralMapBuffer;
};
} // namespace PR
