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

	inline float populationVariance(const Point2i& p, Size1i channel) const;

private:
	const std::shared_ptr<FrameBufferFloat> mOnlineMean;
	const std::shared_ptr<FrameBufferFloat> mOnlineVariance;
};
} // namespace PR

#include "VarianceEstimator.inl"