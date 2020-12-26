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
	var += weight * (value - oldM) * (value - mean);
}

inline void VarianceEstimator::addBlock(Size1i channel,
										const Point2i& global_off,
										const Size2i& global_size,
										const FrameBufferFloat& global_prev_weight_sum,
										const Point2i& local_off,
										const Size2i& local_size,
										const FrameBufferFloat& local_weight, const FrameBufferFloat& local_value)
{
	PR_ASSERT(local_weight.size() == local_value.size(), "Weight and value frame buffers must be the same size");
	PR_ASSERT(global_prev_weight_sum.size() == mOnlineMean->size(), "Previous weight sum and online mean and online variance frame buffers must be the same size");
	PR_ASSERT(global_prev_weight_sum.channels() == 1, "Previous weight sum frame buffer must have only one channel");
	PR_ASSERT(local_weight.channels() == 1, "Local weight frame buffer must have only one channel");

	Point2i global_end = (global_size + global_off).cwiseMin(global_prev_weight_sum.size().asArray());
	Point2i local_end  = (local_size + local_off).cwiseMin(local_weight.size().asArray());

	Point2i size = (local_end - local_off).cwiseMin(global_end - global_off);

	for (Size1i y = 0; y < size.y(); ++y)
		for (Size1i x = 0; x < size.x(); ++x)
			addValue(global_off + Point2i(x, y), channel,
					 global_prev_weight_sum.getFragment(global_off + Point2i(x, y), 0 /*channel*/),
					 local_weight.getFragment(local_off + Point2i(x, y), 0 /*channel*/),
					 local_value.getFragment(local_off + Point2i(x, y), channel));
}
} // namespace PR
