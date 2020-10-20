#pragma once

#include "buffer/FrameBuffer.h"

namespace PR {
class PR_LIB_CORE VarianceEstimator {
public:
	inline VarianceEstimator(const std::shared_ptr<FrameBufferFloat>& onlineM,
							 const std::shared_ptr<FrameBufferFloat>& onlineS,
							 const std::shared_ptr<FrameBufferFloat>& weights);
	inline ~VarianceEstimator();

	inline Size1i channelCount() const;

	inline void addValue(const Point2i& p, Size1i channel, float weight, float value);
	inline void addWeight(const Point2i& p, float weight);

	inline float populationVariance(const Point2i& p, Size1i channel) const;

private:
	const std::shared_ptr<FrameBufferFloat> mOnlineM; // Mean
	const std::shared_ptr<FrameBufferFloat> mOnlineS; // Incremental update value S
	const std::shared_ptr<FrameBufferFloat> mWeights;
};
} // namespace PR

#include "VarianceEstimator.inl"