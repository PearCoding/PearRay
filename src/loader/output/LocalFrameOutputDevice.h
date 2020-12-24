#pragma once

#include "FrameContainer.h"
#include "filter/FilterCache.h"
#include "output/LocalOutputDevice.h"
#include "spectral/SpectralBlob.h"

#include <array>

namespace PR {
class StreamPipeline;
class IFilter;
class PR_LIB_CORE LocalFrameOutputDevice : public LocalOutputDevice {
	friend class FrameOutputDevice;

public:
	explicit LocalFrameOutputDevice(const std::shared_ptr<IFilter>& filter,
									const Size2i& size, Size1i specChannels, bool monotonic);
	virtual ~LocalFrameOutputDevice();

	inline const FrameContainer& data() const { return mData; }
	inline FrameContainer& data() { return mData; }

	inline const Size2i& originalSize() const { return mOriginalSize; }
	inline const Size2i& extendedSize() const { return mExtendedSize; }

	// Mandatory interface
	virtual void clear(bool force = false) override;
	virtual void commitSpectrals(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entrycount) override;
	virtual void commitShadingPoints(const OutputShadingPointEntry* entries, size_t entrycount) override;
	virtual void commitFeedbacks(const OutputFeedbackEntry* entries, size_t entrycount) override;
	virtual void commitCustomSpectrals(uint32 aov_id, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entrycount) override;
	virtual void commitCustom3D(uint32 aov_id, const OutputCustom3DEntry* entries, size_t entrycount) override;
	virtual void commitCustom1D(uint32 aov_id, const OutputCustom1DEntry* entries, size_t entrycount) override;
	virtual void commitCustomCounter(uint32 aov_id, const OutputCustomCounterEntry* entries, size_t entrycount) override;

protected:
	void cache();

private:
	// Using the following function outside the corresponding .cpp will result in a compiler error
	template <bool IsMono, bool HasFilter>
	void commitSpectrals2(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entrycount);
	template <bool IsMono, bool HasFilter>
	void commitCustomSpectrals2(FrameBufferFloat* aov, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entrycount);

	const FilterCache mFilter;
	const Size2i mOriginalSize;
	const Size2i mExtendedSize;
	const bool mMonotonic;
	const bool mHasFilter;

	FrameContainer mData;
	bool mHasNonSpecLPE;
	std::array<std::vector<float>, 3> mSpectralMapBuffer;
};
} // namespace PR
