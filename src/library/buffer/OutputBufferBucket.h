#pragma once

#include "OutputBufferData.h"
#include "spectral/SpectralBlob.h"

namespace PR {
class IFilter;
class PR_LIB OutputBufferBucket {
	friend class OutputBuffer;

public:
	explicit OutputBufferBucket(const std::shared_ptr<IFilter>& filter,
								const Size2i& size, Size1i specChannels);
	~OutputBufferBucket();

	inline const OutputBufferData& data() const { return mData; }

	inline void clear(bool force = false) { mData.clear(force); }

	void pushSpectralFragment(const Point2i& p, const SpectralBlob& spec,
							  const SpectralBlob& wavelengths, bool isMono, const LightPath& path);
	void pushSPFragment(const Point2i& p, const ShadingPoint& pt, const LightPath& path);
	void pushFeedbackFragment(const Point2i& p, uint32 feedback);

	inline const Size2i& originalSize() const { return mOriginalSize; }
	inline const Size2i& viewSize() const { return mViewSize; }
	inline const Size2i& extendedSize() const { return mExtendedSize; }
	inline const Size2i& extendedViewSize() const { return mExtendedViewSize; }

	void shrinkView(const Size2i& newView);

protected:
	void cache();
	inline OutputBufferData& data() { return mData; }

private:
	const std::shared_ptr<IFilter> mFilter;
	const Size2i mOriginalSize;
	const Size2i mExtendedSize;
	Size2i mViewSize;
	Size2i mExtendedViewSize;

	OutputBufferData mData;
	bool mHasNonSpecLPE;
	std::array<std::vector<float>, 3> mSpectralMapBuffer;
};
} // namespace PR
