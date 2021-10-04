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

inline void VarianceEstimator::addValue(const Point2i& p, Size1i channel, float value, size_t iteration)
{
	PR_ASSERT(iteration > 0, "Expected iteration to be larger then zero");

	float& mean = mOnlineMean->getFragment(p, channel);
	float& var	= mOnlineVariance->getFragment(p, channel);

	const float delta = value - mean;
	mean += delta / iteration;
	const float delta2 = value - mean;
	// It would be more numerically stable to use the M2 version of welford's algorithm
	var = (var * (iteration - 1) + delta * delta2) / iteration;
}

inline void VarianceEstimator::addBlock(Size1i channel,
										const Point2i& block_offset,
										const Size2i& block_size,
										const FrameBufferFloat& local_value,
										size_t iteration)
{
	const Point2i block_end = block_size + block_offset;

	for (Size1i y = block_offset.y(); y < block_end.y(); ++y)
		for (Size1i x = block_offset.x(); x < block_end.x(); ++x)
			addValue(Point2i(x, y), channel,
					 local_value.getFragment(Point2i(x, y) - block_offset, channel),
					 iteration);
}
} // namespace PR
