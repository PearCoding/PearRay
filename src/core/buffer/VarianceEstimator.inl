// IWYU pragma: private, include "VarianceEstimator.h"

namespace PR {
inline VarianceEstimator::VarianceEstimator(const std::shared_ptr<FrameBufferFloat>& onlineMean,
											const std::shared_ptr<FrameBufferFloat>& onlineVariance)
	: mOnlineMean(onlineMean)
	, mOnlineVariance(onlineVariance)
{
}

inline VarianceEstimator::~VarianceEstimator() {}

inline Size1i VarianceEstimator::channelCount() const { return mOnlineMean->channels(); }

inline void VarianceEstimator::addValue(const Point2i& p, Size1i channel, float prev_weight_sum, float weight, float weighted_value)
{
	// Do not update if weight is zero
	if (PR_UNLIKELY(weight <= PR_EPSILON))
		return;

	float& mean = mOnlineMean->getFragment(p, channel);
	float& var	= mOnlineVariance->getFragment(p, channel);

	// Due to performance reasons is the value already premultiplied with weight
	const float value  = weighted_value / weight;
	const float oldM   = mean;
	const float newW   = prev_weight_sum + weight;
	const float factor = weight / newW;

	mean += factor * (value - oldM);
	var	 += weight * (value - oldM) * (value - mean);
}

inline float VarianceEstimator::populationVariance(const Point2i& p, Size1i channel) const
{
	return mOnlineVariance->getFragment(p, channel);
}

} // namespace PR
