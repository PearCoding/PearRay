// IWYU pragma: private, include "output/OutputQueue.h"

namespace PR {
inline void OutputQueue::pushSpectralFragment(const Point2i& p, const SpectralBlob& mis, const SpectralBlob& importance, const SpectralBlob& radiance,
											  const SpectralBlob& wavelengths, bool isMono, uint32 rayGroupID, const LightPath& path)
{
	PR_ASSERT(mSpectralIt < mSpectralEntries.size(), "Spectral entries are exhausted");
	const uint32* pathEntry		  = pushPath(path);
	mSpectralEntries[mSpectralIt] = OutputSpectralEntry{ p, mis, importance, radiance, wavelengths, isMono ? (uint32)OSEF_Mono : 0, rayGroupID, pathEntry };
	++mSpectralIt;
}

inline void OutputQueue::pushSPFragment(const Point2i& p, const IntersectionPoint& pt, const LightPath& path)
{
	PR_ASSERT(mSPIt < mSPEntries.size(), "IntersectionPoint entries are exhausted");
	const uint32* pathEntry = pushPath(path);
	mSPEntries[mSPIt]		= OutputShadingPointEntry{ p, pt, pathEntry };
	++mSPIt;
}

inline void OutputQueue::pushFeedbackFragment(const Point2i& p, uint32 feedback)
{
	PR_ASSERT(mFeedbackIt < mFeedbackEntries.size(), "Feedback entries are exhausted");
	mFeedbackEntries[mFeedbackIt] = OutputFeedbackEntry{ p, feedback };
	++mFeedbackIt;
}

inline const uint32* OutputQueue::pushPath(const LightPath& path)
{
	const size_t req_size = path.packedSizeRequirement();
	PR_ASSERT(mLightPathData.usedMemory() + req_size < mLightPathData.maxMemory(), "Lightpath buffer is full");

	uint8* start = (uint8*)mLightPathData.allocate(req_size);
	path.toPacked(start, req_size);
	return (uint32*)start;
}

inline bool OutputQueue::isReadyToCommit() const
{
	return mSpectralIt >= mTriggerThreshold
		   || mSPIt >= mTriggerThreshold
		   || mFeedbackIt >= mTriggerThreshold
		   || mLightPathData.usedMemory() >= mMemTriggerThreshold;
}
} // namespace PR