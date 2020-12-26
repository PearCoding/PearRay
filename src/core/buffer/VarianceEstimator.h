#pragma once

#include "buffer/FrameBuffer.h"

namespace PR {
class PR_LIB_CORE VarianceEstimator {
public:
	inline VarianceEstimator(const std::shared_ptr<FrameBufferFloat>& onlineMean,
							 const std::shared_ptr<FrameBufferFloat>& onlineVariance);
	inline ~VarianceEstimator();

	inline Size1i channelCount() const;

	inline void addValue(const Point2i& p, Size1i channel, float prev_weight_sum, float weight, float value);

	/// This will be used in local to global merge
	/// prev_weight_sum and the variance estimator are global, weight and value are local
	inline void addBlock(Size1i channel,
						 const Point2i& global_off,
						 const Size2i& global_size,
						 const FrameBufferFloat& global_prev_weight_sum,
						 const Point2i& local_off,
						 const Size2i& local_size,
						 const FrameBufferFloat& local_weight, const FrameBufferFloat& local_value);

private:
	const std::shared_ptr<FrameBufferFloat> mOnlineMean;
	const std::shared_ptr<FrameBufferFloat> mOnlineVariance;
};
} // namespace PR

#include "VarianceEstimator.inl"