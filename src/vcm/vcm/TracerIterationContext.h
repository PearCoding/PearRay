#pragma once

#include "Kernel.h"
#include "Options.h"

namespace PR {
class RenderTileSession;

namespace VCM {
class TracerThreadContext;

// Data for one thread
template <bool UseMerging, MISMode Mode>
class TracerIterationContext {
public:
	inline TracerIterationContext(size_t iteration, float maxSceneGatherRadius, RenderTileSession& session, TracerThreadContext& threadContext, const Options& options)
		: Radius(std::max<float>(0.000001f, maxSceneGatherRadius / std::pow<float>(iteration + 1, 0.5f * (1 - options.ContractRatio))))
		, Radius2(Radius * Radius)
		, VMNormalization(1 / (kernelarea(Radius2) * options.MaxLightSamples))
		, _MISVMWeightFactor(mis_term<Mode>(1 / VMNormalization))
		, _MISVCWeightFactor(mis_term<Mode>(VMNormalization))
		, Session(session)
		, ThreadContext(threadContext)
	{
	}

	template <bool UM = UseMerging>
	inline TracerIterationContext(RenderTileSession& session, TracerThreadContext& threadContext, const Options& options,
								  typename std::enable_if<!UM>::type* = 0)
		: TracerIterationContext(0, 0.0f, session, threadContext, options)
	{
	}

	const float Radius;
	const float Radius2;
	const float VMNormalization;
	const float _MISVMWeightFactor;
	const float _MISVCWeightFactor;

	inline float MISVMWeightFactor() const
	{
		if constexpr (UseMerging)
			return _MISVMWeightFactor;
		else
			return 0.0f;
	}

	inline float MISVCWeightFactor() const
	{
		if constexpr (UseMerging)
			return _MISVCWeightFactor;
		else
			return 0.0f;
	}

	// Context of evaluation
	RenderTileSession& Session;
	TracerThreadContext& ThreadContext;
};
} // namespace VCM
} // namespace PR