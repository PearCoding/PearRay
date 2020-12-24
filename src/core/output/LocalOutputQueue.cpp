#include "LocalOutputQueue.h"
#include "LocalOutputSystem.h"
#include "OutputSystem.h"
#include "Profiler.h"

namespace PR {
constexpr size_t AVG_PATH_LENGTH = 8;
constexpr float MEM_SAFE_FACTOR	 = 0.9f;
LocalOutputQueue::LocalOutputQueue(OutputSystem* system, StreamPipeline* pipeline, size_t max_entries, size_t trigger_threshold)
	: mSystem(system)
	, mSpectralQueue(max_entries)
	, mSPQueue(max_entries)
	, mFeedbackQueue(max_entries)
	, mLightPathData(max_entries * (AVG_PATH_LENGTH + 1) * sizeof(uint32))
	, mPipeline(pipeline)
	, mMaxEntries(max_entries)
	, mTriggerThreshold(std::min(trigger_threshold, max_entries))
	, mMemTriggerThreshold(mLightPathData.maxMemory() * MEM_SAFE_FACTOR)
{
	// TODO: Better get the maximum entry in the map and use that!
	for (size_t i = 0; i < system->customSpectralChannels().size(); ++i)
		mCustomSpectralQueues.emplace_back(max_entries);
	for (size_t i = 0; i < system->custom1DChannels().size(); ++i)
		mCustom1DQueues.emplace_back(max_entries);
	for (size_t i = 0; i < system->custom3DChannels().size(); ++i)
		mCustom3DQueues.emplace_back(max_entries);
	for (size_t i = 0; i < system->customCounterChannels().size(); ++i)
		mCustomCounterQueues.emplace_back(max_entries);
}

LocalOutputQueue::~LocalOutputQueue()
{
}

void LocalOutputQueue::commitTo(LocalOutputSystem* system) const
{
	PR_PROFILE_THIS;

	system->commitSpectrals(mPipeline, mSpectralQueue.Entries.data(), mSpectralQueue.It);
	system->commitShadingPoints(mSPQueue.Entries.data(), mSPQueue.It);
	system->commitFeedbacks(mFeedbackQueue.Entries.data(), mFeedbackQueue.It);

	for (const auto& p : mSystem->customSpectralChannels()) {
		const auto& queue = mCustomSpectralQueues[p.second];
		system->commitCustomSpectrals(p.second, mPipeline, queue.Entries.data(), queue.It);
	}

	for (const auto& p : mSystem->custom1DChannels()) {
		const auto& queue = mCustom1DQueues[p.second];
		system->commitCustom1D(p.second, queue.Entries.data(), queue.It);
	}

	for (const auto& p : mSystem->custom3DChannels()) {
		const auto& queue = mCustom3DQueues[p.second];
		system->commitCustom3D(p.second, queue.Entries.data(), queue.It);
	}

	for (const auto& p : mSystem->customCounterChannels()) {
		const auto& queue = mCustomCounterQueues[p.second];
		system->commitCustomCounter(p.second, queue.Entries.data(), queue.It);
	}

	++mCommitCount;
}

void LocalOutputQueue::flush()
{
	mSpectralQueue.It = 0;
	mSPQueue.It		  = 0;
	mFeedbackQueue.It = 0;

	for (auto& q : mCustomSpectralQueues)
		q.It = 0;
	for (auto& q : mCustom3DQueues)
		q.It = 0;
	for (auto& q : mCustom1DQueues)
		q.It = 0;
	for (auto& q : mCustomCounterQueues)
		q.It = 0;

	mLightPathData.freeAll();

	++mFlushCount;
}
} // namespace PR