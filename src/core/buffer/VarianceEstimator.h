#pragma once

#include "buffer/FrameBuffer.h"

namespace PR {
class PR_LIB_CORE VarianceEstimator {
public:
	inline VarianceEstimator(const std::shared_ptr<FrameBufferFloat>& onlineMean,
							 const std::shared_ptr<FrameBufferFloat>& onlineVariance);
	inline ~VarianceEstimator();

	inline Size1i channelCount() const;

	inline void addValue(const Point2i& p, Size1i channel, float value, size_t iteration);

	/// This will be used in local to global merge
	/// prev_weight_sum and the variance estimator are global, weight and value are local
	/// @param iteration the current iteration the local values are from
	inline void addBlock(Size1i channel,
						 const Point2i& global_off,
						 const Size2i& global_size,
						 const Point2i& local_off,
						 const Size2i& local_size,
						 const FrameBufferFloat& local_value,
						 size_t iteration);

private:
	const std::shared_ptr<FrameBufferFloat> mOnlineMean;
	const std::shared_ptr<FrameBufferFloat> mOnlineVariance;
};
} // namespace PR

#include "VarianceEstimator.inl"