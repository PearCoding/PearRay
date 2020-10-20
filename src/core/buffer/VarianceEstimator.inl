// IWYU pragma: private, include "VarianceEstimator.h"

namespace PR {
inline VarianceEstimator::VarianceEstimator(const std::shared_ptr<FrameBufferFloat>& onlineM,
											const std::shared_ptr<FrameBufferFloat>& onlineS,
											const std::shared_ptr<FrameBufferFloat>& weights)
	: mOnlineM(onlineM)
	, mOnlineS(onlineS)
	, mWeights(weights)
{
}

inline VarianceEstimator::~VarianceEstimator() {}

inline Size1i VarianceEstimator::channelCount() const { return mOnlineM->channels(); }

inline void VarianceEstimator::addValue(const Point2i& p, Size1i channel, float weight, float value)
{
	float& mean		 = mOnlineM->getFragment(p, channel);
	const float oldM = mean;
	const float newW = mWeights->getFragment(p, 0) + weight;

	mean += (weight / newW) * (value - oldM);									   // Update mean
	mOnlineS->getFragment(p, channel) += weight * (value - oldM) * (value - mean); // Update S
}

inline void VarianceEstimator::addWeight(const Point2i& p, float weight)
{
	mWeights->getFragment(p, 0) += weight;
}

inline float VarianceEstimator::populationVariance(const Point2i& p, Size1i channel) const
{
	return mOnlineS->getFragment(p, channel) / mWeights->getFragment(p, 0);
}

} // namespace PR
