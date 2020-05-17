#pragma once

#include "FrameBufferContainer.h"
#include "output/OutputDevice.h"
#include <mutex>

namespace PR {
class IFilter;
class FrameBufferBucket;
class PR_LIB_LOADER FrameOutputDevice : public OutputDevice {
public:
	explicit FrameOutputDevice(Size1i specChannels);
	~FrameOutputDevice();

	inline FrameBufferContainer& data() { return *mData; }
	inline const FrameBufferContainer& data() const { return *mData; }

	inline void clear(bool force = false) { mData->clear(force); }

	// OutputDevice interface
	void commitSpectrals(const OutputCommitInformation& info, const OutputSpectralEntry* entries, size_t entrycount) override;
	void commitShadingPoints(const OutputCommitInformation& info, const OutputShadingPointEntry* entries, size_t entrycount) override;
	void commitFeedbacks(const OutputCommitInformation& info, const OutputFeedbackEntry* entries, size_t entrycount) override;

	void onStart(RenderContext* ctx);
	void onNextIteration();
	void onStop();

protected:
	virtual void handleSpectrals(const OutputSpectralEntry* entries, size_t entrycount) = 0;

private:
	const Size1i mSpectralChannels;

	std::vector<std::shared_ptr<FrameBufferBucket>> mBuckets;
	std::unique_ptr<FrameBufferContainer> mData;
	std::mutex mMergeMutex;
};
} // namespace PR
