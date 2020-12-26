// IWYU pragma: private, include "FrameContainer.h"

namespace PR {
// Internal
inline bool FrameContainer::hasInternalChannel_1D(AOV1D var) const { return getInternalChannel_1D(var) != nullptr; }
inline bool FrameContainer::hasInternalChannel_Counter(AOVCounter var) const { return getInternalChannel_Counter(var) != nullptr; }
inline bool FrameContainer::hasInternalChannel_3D(AOV3D var) const { return getInternalChannel_3D(var) != nullptr; }
inline bool FrameContainer::hasInternalChannel_Spectral(AOVSpectral var) const { return getInternalChannel_Spectral(var) != nullptr; }

inline std::shared_ptr<FrameBufferFloat> FrameContainer::getInternalChannel_1D(AOV1D var) const { return mInt1D[var]; }
inline std::shared_ptr<FrameBufferUInt32> FrameContainer::getInternalChannel_Counter(AOVCounter var) const { return mIntCounter[var]; }
inline std::shared_ptr<FrameBufferFloat> FrameContainer::getInternalChannel_3D(AOV3D var) const { return mInt3D[var]; }
inline std::shared_ptr<FrameBufferFloat> FrameContainer::getInternalChannel_Spectral(AOVSpectral var) const { return mSpectral[var]; }

inline bool FrameContainer::hasVarianceEstimator() const
{
	return mSpectral[AOV_OnlineMean] && mSpectral[AOV_OnlineVariance];
}

inline VarianceEstimator FrameContainer::varianceEstimator() const
{
	PR_ASSERT(hasVarianceEstimator(), "Expected to have a variance estimator");
	return VarianceEstimator(mSpectral[AOV_OnlineMean], mSpectral[AOV_OnlineVariance]);
}

inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestInternalChannel_1D(AOV1D var)
{
	if (mInt1D[var])
		return mInt1D[var];
	auto output = create1DBuffer();
	mInt1D[var] = output;
	return output;
}

inline std::shared_ptr<FrameBufferUInt32> FrameContainer::requestInternalChannel_Counter(AOVCounter var)
{
	if (mIntCounter[var])
		return mIntCounter[var];
	auto output		 = createCounterBuffer();
	mIntCounter[var] = output;
	return output;
}

inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestInternalChannel_3D(AOV3D var)
{
	if (mInt3D[var])
		return mInt3D[var];
	auto output = create3DBuffer();
	mInt3D[var] = output;
	return output;
}

inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestInternalChannel_Spectral(AOVSpectral var)
{
	if (mSpectral[var])
		return mSpectral[var];
	auto output	   = createSpectralBuffer();
	mSpectral[var] = output;
	return output;
}

// Custom
inline bool FrameContainer::hasCustomChannel_1D(uint32 id) const { return id < mCustom1D.size(); }
inline bool FrameContainer::hasCustomChannel_Counter(uint32 id) const { return id < mCustomCounter.size(); }
inline bool FrameContainer::hasCustomChannel_3D(uint32 id) const { return id < mCustom3D.size(); }
inline bool FrameContainer::hasCustomChannel_Spectral(uint32 id) const { return id < mCustomSpectral.size(); }

inline std::shared_ptr<FrameBufferFloat> FrameContainer::getCustomChannel_1D(uint32 id) const
{
	return hasCustomChannel_1D(id) ? mCustom1D[id] : nullptr;
}
inline std::shared_ptr<FrameBufferUInt32> FrameContainer::getCustomChannel_Counter(uint32 id) const
{
	return hasCustomChannel_Counter(id) ? mCustomCounter[id] : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::getCustomChannel_3D(uint32 id) const
{
	return hasCustomChannel_3D(id) ? mCustom3D[id] : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::getCustomChannel_Spectral(uint32 id) const
{
	return hasCustomChannel_Spectral(id) ? mCustomSpectral[id] : nullptr;
}

inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestCustomChannel_1D(uint32 id)
{
	if (hasCustomChannel_1D(id))
		return mCustom1D[id];

	// Resize such that the id is valid
	mCustom1D.resize(id + 1);

	auto output	  = create1DBuffer();
	mCustom1D[id] = output;
	return output;
}
inline std::shared_ptr<FrameBufferUInt32> FrameContainer::requestCustomChannel_Counter(uint32 id)
{
	if (hasCustomChannel_Counter(id))
		return mCustomCounter[id];

	// Resize such that the id is valid
	mCustomCounter.resize(id + 1);

	auto output		   = createCounterBuffer();
	mCustomCounter[id] = output;
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestCustomChannel_3D(uint32 id)
{
	if (hasCustomChannel_3D(id))
		return mCustom3D[id];

	// Resize such that the id is valid
	mCustom3D.resize(id + 1);

	auto output	  = create3DBuffer();
	mCustom3D[id] = output;
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestCustomChannel_Spectral(uint32 id)
{
	if (hasCustomChannel_Spectral(id))
		return mCustomSpectral[id];

	// Resize such that the id is valid
	mCustomSpectral.resize(id + 1);

	auto output			= createSpectralBuffer();
	mCustomSpectral[id] = output;
	return output;
}

// LPE
inline size_t FrameContainer::getLPEChannelCount_1D(AOV1D var) const { return mLPE_1D[var].size(); }
inline size_t FrameContainer::getLPEChannelCount_Counter(AOVCounter var) const { return mLPE_Counter[var].size(); }
inline size_t FrameContainer::getLPEChannelCount_3D(AOV3D var) const { return mLPE_3D[var].size(); }
inline size_t FrameContainer::getLPEChannelCount_Spectral(AOVSpectral var) const { return mLPE_Spectral[var].size(); }

inline std::shared_ptr<FrameBufferFloat> FrameContainer::getLPEChannel_1D(AOV1D var, size_t i) const
{
	return i < getLPEChannelCount_1D(var) ? mLPE_1D[var].at(i).second : nullptr;
}
inline std::shared_ptr<FrameBufferUInt32> FrameContainer::getLPEChannel_Counter(AOVCounter var, size_t i) const
{
	return i < getLPEChannelCount_Counter(var) ? mLPE_Counter[var].at(i).second : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::getLPEChannel_3D(AOV3D var, size_t i) const
{
	return i < getLPEChannelCount_3D(var) ? mLPE_3D[var].at(i).second : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::getLPEChannel_Spectral(AOVSpectral var, size_t i) const
{
	return i < getLPEChannelCount_Spectral(var) ? mLPE_Spectral[var].at(i).second : nullptr;
}

inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestLPEChannel_1D(AOV1D var, const LightPathExpression& expr, uint32 id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = create1DBuffer();
	if (id >= mLPE_1D[var].size())
		mLPE_1D[var].resize(id);
	mLPE_1D[var].emplace_back(std::make_pair(expr, output));
	return output;
}
inline std::shared_ptr<FrameBufferUInt32> FrameContainer::requestLPEChannel_Counter(AOVCounter var, const LightPathExpression& expr, uint32 id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = createCounterBuffer();
	if (id >= mLPE_Counter[var].size())
		mLPE_Counter[var].resize(id);
	mLPE_Counter[var].emplace_back(std::make_pair(expr, output));
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestLPEChannel_3D(AOV3D var, const LightPathExpression& expr, uint32 id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = create3DBuffer();
	if (id >= mLPE_3D[var].size())
		mLPE_3D[var].resize(id);
	mLPE_3D[var].emplace_back(std::make_pair(expr, output));
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameContainer::requestLPEChannel_Spectral(AOVSpectral var, const LightPathExpression& expr, uint32 id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = createSpectralBuffer();
	if (id >= mLPE_Spectral[var].size())
		mLPE_Spectral[var].resize(id);
	mLPE_Spectral[var].emplace_back(std::make_pair(expr, output));
	return output;
}

} // namespace PR
