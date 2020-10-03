#include "OutputQueue.h"
#include "Profiler.h"
#include "buffer/FrameBufferBucket.h"

namespace PR {
constexpr size_t AVG_PATH_LENGTH = 8;
constexpr float MEM_SAFE_FACTOR	 = 0.9f;
OutputQueue::OutputQueue(StreamPipeline* pipeline, size_t max_entries, size_t trigger_threshold)
	: mSpectralEntries(max_entries)
	, mSpectralIt(0)
	, mSPEntries(max_entries)
	, mSPIt(0)
	, mFeedbackEntries(max_entries)
	, mFeedbackIt(0)
	, mLightPathData(max_entries * (AVG_PATH_LENGTH + 1) * sizeof(uint32))
	, mPipeline(pipeline)
	, mTriggerThreshold(std::min(trigger_threshold, max_entries))
	, mMemTriggerThreshold(mLightPathData.maxMemory() * MEM_SAFE_FACTOR)
{
}

OutputQueue::~OutputQueue()
{
}

void OutputQueue::commitTo(OutputDevice* device) const
{
	PR_PROFILE_THIS;
	PR_UNUSED(device);
	// TODO
}

void OutputQueue::commitTo(FrameBufferBucket* bucket) const
{
	PR_PROFILE_THIS;

	bucket->commitSpectrals(mPipeline, mSpectralEntries.data(), mSpectralIt);
	bucket->commitShadingPoints(mSPEntries.data(), mSPIt);
	bucket->commitFeedbacks(mFeedbackEntries.data(), mFeedbackIt);

	for (auto callback : mSpectralCallbacks)
		callback(mSpectralEntries.data(), mSpectralIt);
}

void OutputQueue::flush()
{
	mSpectralIt = 0;
	mSPIt		= 0;
	mFeedbackIt = 0;
	mLightPathData.freeAll();
}
} // namespace PR