// IWYU pragma: private, include "output/LocalOutputQueue.h"

#ifdef PR_DEBUG
#define PR_CHECK_NANS
#define PR_CHECK_INFS
#define PR_CHECK_NEGS
#endif
namespace PR {
inline void LocalOutputQueue::pushSpectralFragment(const Point2i& p, float mis, const SpectralBlob& importance, const SpectralBlob& radiance,
												   const SpectralBlob& wavelengths, bool isMono, uint32 rayGroupID, const LightPath& path)
{
#ifdef PR_CHECK_NANS
	PR_ASSERT(!std::isnan(mis), "Given MIS term has NaNs");
	PR_ASSERT(!importance.hasNaN(), "Given Importance term has NaNs");
	PR_ASSERT(!radiance.hasNaN(), "Given Radiance term has NaNs");
#endif
#ifdef PR_CHECK_INFS
	PR_ASSERT(!std::isinf(mis), "Given MIS term has Infs");
	PR_ASSERT(!importance.isInf().any(), "Given Importance term has Infs");
	PR_ASSERT(!radiance.isInf().any(), "Given Radiance term has Infs");
#endif
#ifdef PR_CHECK_NEGS
	PR_ASSERT(!(mis < -PR_EPSILON), "Given MIS term has Negatives");
	PR_ASSERT(!((importance < -PR_EPSILON).any()), "Given Importance term has Negatives");
	PR_ASSERT(!((radiance < -PR_EPSILON).any()), "Given Radiance term has Negatives");
#endif
	PR_ASSERT(!mSpectralQueue.isFull(), "Spectral entries are exhausted");
	const uint32* pathEntry = pushPath(path);
	mSpectralQueue.add(OutputSpectralEntry{ p, mis, importance, radiance, wavelengths, isMono ? (uint32)OSEF_Mono : 0, rayGroupID, pathEntry });
}

inline void LocalOutputQueue::pushSPFragment(const Point2i& p, const IntersectionPoint& pt, const LightPath& path)
{
	PR_ASSERT(!mSPQueue.isFull(), "IntersectionPoint entries are exhausted");
	const uint32* pathEntry = pushPath(path);
	mSPQueue.add(OutputShadingPointEntry{ p, pt, pathEntry });
}

inline void LocalOutputQueue::pushFeedbackFragment(const Point2i& p, uint32 feedback)
{
	PR_ASSERT(!mFeedbackQueue.isFull(), "Feedback entries are exhausted");
	mFeedbackQueue.add(OutputFeedbackEntry{ p, feedback });
}

inline void LocalOutputQueue::pushCustomSpectralFragment(uint32 queueID, const Point2i& p, const SpectralBlob& value, const SpectralBlob& wavelengths, bool isMono, uint32 rayGroupID)
{
	PR_ASSERT(queueID < mCustomSpectralQueues.size(), "Expected valid custom spectral queueID");
	PR_ASSERT(!mCustomSpectralQueues[queueID].isFull(), "Custom spectral entries are exhausted");
	mCustomSpectralQueues[queueID].add(OutputCustomSpectralEntry{ { p, value }, wavelengths, isMono ? (uint32)OSEF_Mono : 0, rayGroupID });
}

inline void LocalOutputQueue::pushCustom3DFragment(uint32 queueID, const Point2i& p, const Vector3f& value)
{
	PR_ASSERT(queueID < mCustom3DQueues.size(), "Expected valid custom 3d queueID");
	PR_ASSERT(!mCustom3DQueues[queueID].isFull(), "Custom 3d entries are exhausted");
	mCustom3DQueues[queueID].add(OutputCustom3DEntry{ p, value });
}

inline void LocalOutputQueue::pushCustom1DFragment(uint32 queueID, const Point2i& p, float value)
{
	PR_ASSERT(queueID < mCustom1DQueues.size(), "Expected valid custom 1d queueID");
	PR_ASSERT(!mCustom1DQueues[queueID].isFull(), "Custom 1d entries are exhausted");
	mCustom1DQueues[queueID].add(OutputCustom1DEntry{ p, value });
}

inline void LocalOutputQueue::pushCustomCounterFragment(uint32 queueID, const Point2i& p, uint32 value)
{
	PR_ASSERT(queueID < mCustomCounterQueues.size(), "Expected valid custom counter queueID");
	PR_ASSERT(!mCustomCounterQueues[queueID].isFull(), "Custom counter entries are exhausted");
	mCustomCounterQueues[queueID].add(OutputCustomCounterEntry{ p, value });
}

inline const uint32* LocalOutputQueue::pushPath(const LightPath& path)
{
	const size_t req_size = path.packedSizeRequirement();
	PR_ASSERT(mLightPathData.usedMemory() + req_size < mLightPathData.maxMemory(), "Lightpath buffer is full");

	uint8* start = (uint8*)mLightPathData.allocate(req_size);
	path.toPacked(start, req_size);
	return (uint32*)start;
}

inline bool LocalOutputQueue::isReadyToCommit() const
{
	if (mSpectralQueue.isReady(mTriggerThreshold))
		return true;

	if (mSPQueue.isReady(mTriggerThreshold))
		return true;

	if (mFeedbackQueue.isReady(mTriggerThreshold))
		return true;

	if (mLightPathData.usedMemory() >= mMemTriggerThreshold)
		return true;

	for (const auto& q : mCustomSpectralQueues)
		if (q.isReady(mTriggerThreshold))
			return true;

	for (const auto& q : mCustom3DQueues)
		if (q.isReady(mTriggerThreshold))
			return true;

	for (const auto& q : mCustom1DQueues)
		if (q.isReady(mTriggerThreshold))
			return true;

	for (const auto& q : mCustomCounterQueues)
		if (q.isReady(mTriggerThreshold))
			return true;

	return false;
}
} // namespace PR