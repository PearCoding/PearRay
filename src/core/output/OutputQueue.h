#pragma once

#include "OutputData.h"
#include "memory/MemoryStack.h"
#include "path/LightPath.h"

namespace PR {
class OutputDevice;
class FrameBufferBucket;
class StreamPipeline;

using OutputQueueSpectralCallback = std::function<void(const OutputSpectralEntry*, size_t)>;
class PR_LIB_CORE OutputQueue {
public:
	OutputQueue(StreamPipeline* pipeline, size_t max_entries, size_t trigger_threshold);
	~OutputQueue();

	inline void pushSpectralFragment(const Point2i& p, const SpectralBlob& mis, const SpectralBlob& importance, const SpectralBlob& radiance,
									 const SpectralBlob& wavelengths, bool isMono, uint32 rayGroupID, const LightPath& path);
	inline void pushSPFragment(const Point2i& p, const IntersectionPoint& pt, const LightPath& path);
	inline void pushFeedbackFragment(const Point2i& p, uint32 feedback);

	inline bool isReadyToCommit() const;
	void commitTo(OutputDevice* device) const;
	void commitTo(FrameBufferBucket* bucket) const;
	void flush();

	inline void commitAndFlush(FrameBufferBucket* bucket)
	{
		commitTo(bucket);
		flush();
	}

	inline void registerSpectralCallback(const OutputQueueSpectralCallback& callback)
	{
		mSpectralCallbacks.push_back(callback);
	}

	/*inline void unregisterSpectralCallback(const OutputQueueSpectralCallback& callback)
	{
		mSpectralCallbacks.erase(std::remove(mSpectralCallbacks.begin(), mSpectralCallbacks.end(), callback));
	}*/

private:
	inline const uint32* pushPath(const LightPath& path);

	std::vector<OutputSpectralEntry, Eigen::aligned_allocator<OutputSpectralEntry>> mSpectralEntries;
	size_t mSpectralIt;
	std::vector<OutputQueueSpectralCallback> mSpectralCallbacks;

	std::vector<OutputShadingPointEntry, Eigen::aligned_allocator<OutputShadingPointEntry>> mSPEntries;
	size_t mSPIt;

	std::vector<OutputFeedbackEntry, Eigen::aligned_allocator<OutputFeedbackEntry>> mFeedbackEntries;
	size_t mFeedbackIt;

	MemoryStack mLightPathData;

	StreamPipeline* mPipeline;

	const size_t mTriggerThreshold;
	const size_t mMemTriggerThreshold;
};
} // namespace PR

#include "OutputQueue.inl"