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
										const Point2i& global_off,
										const Size2i& global_size,
										const Point2i& local_off,
										const Size2i& local_size,
										const FrameBufferFloat& local_value,
										size_t iteration)
{
	Point2i global_end = (global_size + global_off).cwiseMin(mOnlineMean->size().asArray());
	Point2i local_end  = (local_size + local_off).cwiseMin(local_value.size().asArray());

	Point2i size = (local_end - local_off).cwiseMin(global_end - global_off);

	for (Size1i y = 0; y < size.y(); ++y)
		for (Size1i x = 0; x < size.x(); ++x)
			addValue(global_off + Point2i(x, y), channel,
					 local_value.getFragment(local_off + Point2i(x, y), channel),
					 iteration);
}
} // namespace PR
