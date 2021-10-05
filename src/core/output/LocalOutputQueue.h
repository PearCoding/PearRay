#pragma once

#include "OutputData.h"
#include "memory/MemoryStack.h"
#include "path/LightPath.h"

#include <atomic>

namespace PR {
class LocalOutputSystem;
class OutputSystem;
class StreamPipeline;

class PR_LIB_CORE LocalOutputQueue {
public:
	LocalOutputQueue(OutputSystem* system, StreamPipeline* pipeline, size_t max_entries, size_t trigger_threshold);
	~LocalOutputQueue();

	inline void pushSpectralFragment(const Point2i& p, const SpectralBlob& mis, const SpectralBlob& importance, const SpectralBlob& radiance,
									 const SpectralBlob& wavelengths, bool isMono, uint32 rayGroupID, const LightPath& path);
	inline void pushSPFragment(const Point2i& p, const IntersectionPoint& pt, const LightPath& path);
	inline void pushFeedbackFragment(const Point2i& p, uint32 feedback);

	inline void pushCustomSpectralFragment(uint32 queueID, const Point2i& p, const SpectralBlob& value, const SpectralBlob& wavelengths, bool isMono, uint32 rayGroupID);
	inline void pushCustom3DFragment(uint32 queueID, const Point2i& p, const Vector3f& value);
	inline void pushCustom1DFragment(uint32 queueID, const Point2i& p, float value);
	inline void pushCustomCounterFragment(uint32 queueID, const Point2i& p, uint32 value);

	inline bool isReadyToCommit() const;

	void commitTo(LocalOutputSystem* localSystem) const;
	void flush();

	inline void commitAndFlush(LocalOutputSystem* localSystem)
	{
		commitTo(localSystem);
		flush();
	}

	inline size_t commitCount() const { return mCommitCount; }
	inline size_t flushCount() const { return mFlushCount; }

	inline size_t maxEntries() const { return mMaxEntries; }
	inline size_t entryTriggerThreshold() const { return mTriggerThreshold; }
	inline size_t memoryTriggerThreshold() const { return mMemTriggerThreshold; }

private:
	inline const uint32* pushPath(const LightPath& path);

	template <typename T>
	struct Queue {
		std::vector<T, Eigen::aligned_allocator<T>> Entries;
		size_t It;

		inline explicit Queue(size_t entries)
			: Entries(entries)
			, It(0)
		{
		}

		inline bool isFull() const { return It >= Entries.size(); }
		inline bool isReady(size_t threshold) const { return It >= threshold; }

		inline void add(const T& val)
		{
			Entries[It++] = val;
		}
	};

	const OutputSystem* mSystem;

	Queue<OutputSpectralEntry> mSpectralQueue;
	Queue<OutputShadingPointEntry> mSPQueue;
	Queue<OutputFeedbackEntry> mFeedbackQueue;

	std::vector<Queue<OutputCustomSpectralEntry>> mCustomSpectralQueues;
	std::vector<Queue<OutputCustom3DEntry>> mCustom3DQueues;
	std::vector<Queue<OutputCustom1DEntry>> mCustom1DQueues;
	std::vector<Queue<OutputCustomCounterEntry>> mCustomCounterQueues;

	MemoryStack mLightPathData;

	StreamPipeline* mPipeline;

	// Statistics
	mutable std::atomic<size_t> mCommitCount;
	mutable std::atomic<size_t> mFlushCount;

	const size_t mMaxEntries;
	const size_t mTriggerThreshold;
	const size_t mMemTriggerThreshold;
};
} // namespace PR

#include "LocalOutputQueue.inl"