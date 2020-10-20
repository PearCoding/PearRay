// IWYU pragma: private, include "FrameBufferContainer.h"

namespace PR {
// Internal
inline bool FrameBufferContainer::hasInternalChannel_1D(AOV1D var) const { return getInternalChannel_1D(var) != nullptr; }
inline bool FrameBufferContainer::hasInternalChannel_Counter(AOVCounter var) const { return getInternalChannel_Counter(var) != nullptr; }
inline bool FrameBufferContainer::hasInternalChannel_3D(AOV3D var) const { return getInternalChannel_3D(var) != nullptr; }
inline bool FrameBufferContainer::hasInternalChannel_Spectral(AOVSpectral var) const { return getInternalChannel_Spectral(var) != nullptr; }

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getInternalChannel_1D(AOV1D var) const { return mInt1D[var]; }
inline std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::getInternalChannel_Counter(AOVCounter var) const { return mIntCounter[var]; }
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getInternalChannel_3D(AOV3D var) const { return mInt3D[var]; }
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getInternalChannel_Spectral(AOVSpectral var) const { return mSpectral[var]; }

inline VarianceEstimator FrameBufferContainer::varianceEstimator() const
{
	return VarianceEstimator(mSpectral[AOV_OnlineM], mSpectral[AOV_OnlineS], mInt1D[AOV_PixelWeight]);
}

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestInternalChannel_1D(AOV1D var)
{
	if (mInt1D[var])
		return mInt1D[var];
	auto output = create1DBuffer();
	mInt1D[var] = output;
	return output;
}

inline std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::requestInternalChannel_Counter(AOVCounter var)
{
	if (mIntCounter[var])
		return mIntCounter[var];
	auto output		 = createCounterBuffer();
	mIntCounter[var] = output;
	return output;
}

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestInternalChannel_3D(AOV3D var)
{
	if (mInt3D[var])
		return mInt3D[var];
	auto output = create3DBuffer();
	mInt3D[var] = output;
	return output;
}

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestInternalChannel_Spectral(AOVSpectral var)
{
	if (mSpectral[var])
		return mSpectral[var];
	auto output	   = createSpectralBuffer();
	mSpectral[var] = output;
	return output;
}

// Custom
inline bool FrameBufferContainer::hasCustomChannel_1D(const std::string& str) const { return mCustom1D.count(str) > 0; }
inline bool FrameBufferContainer::hasCustomChannel_Counter(const std::string& str) const { return mCustomCounter.count(str) > 0; }
inline bool FrameBufferContainer::hasCustomChannel_3D(const std::string& str) const { return mCustom3D.count(str) > 0; }
inline bool FrameBufferContainer::hasCustomChannel_Spectral(const std::string& str) const { return mCustomSpectral.count(str) > 0; }

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getCustomChannel_1D(const std::string& str) const
{
	return hasCustomChannel_1D(str) ? mCustom1D.at(str) : nullptr;
}
inline std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::getCustomChannel_Counter(const std::string& str) const
{
	return hasCustomChannel_Counter(str) ? mCustomCounter.at(str) : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getCustomChannel_3D(const std::string& str) const
{
	return hasCustomChannel_3D(str) ? mCustom3D.at(str) : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getCustomChannel_Spectral(const std::string& str) const
{
	return hasCustomChannel_Spectral(str) ? mCustomSpectral.at(str) : nullptr;
}

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestCustomChannel_1D(const std::string& str)
{
	if (hasCustomChannel_1D(str))
		return mCustom1D.at(str);

	auto output	   = create1DBuffer();
	mCustom1D[str] = output;
	return output;
}
inline std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::requestCustomChannel_Counter(const std::string& str)
{
	if (hasCustomChannel_Counter(str))
		return mCustomCounter.at(str);

	auto output			= createCounterBuffer();
	mCustomCounter[str] = output;
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestCustomChannel_3D(const std::string& str)
{
	if (hasCustomChannel_3D(str))
		return mCustom3D.at(str);

	auto output	   = create3DBuffer();
	mCustom3D[str] = output;
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestCustomChannel_Spectral(const std::string& str)
{
	if (hasCustomChannel_Spectral(str))
		return mCustomSpectral.at(str);

	auto output			 = createSpectralBuffer();
	mCustomSpectral[str] = output;
	return output;
}

// LPE
inline size_t FrameBufferContainer::getLPEChannelCount_1D(AOV1D var) const { return mLPE_1D[var].size(); }
inline size_t FrameBufferContainer::getLPEChannelCount_Counter(AOVCounter var) const { return mLPE_Counter[var].size(); }
inline size_t FrameBufferContainer::getLPEChannelCount_3D(AOV3D var) const { return mLPE_3D[var].size(); }
inline size_t FrameBufferContainer::getLPEChannelCount_Spectral(AOVSpectral var) const { return mLPE_Spectral[var].size(); }

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getLPEChannel_1D(AOV1D var, size_t i) const
{
	return i < getLPEChannelCount_1D(var) ? mLPE_1D[var].at(i).second : nullptr;
}
inline std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::getLPEChannel_Counter(AOVCounter var, size_t i) const
{
	return i < getLPEChannelCount_Counter(var) ? mLPE_Counter[var].at(i).second : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getLPEChannel_3D(AOV3D var, size_t i) const
{
	return i < getLPEChannelCount_3D(var) ? mLPE_3D[var].at(i).second : nullptr;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::getLPEChannel_Spectral(AOVSpectral var, size_t i) const
{
	return i < getLPEChannelCount_Spectral(var) ? mLPE_Spectral[var].at(i).second : nullptr;
}

inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestLPEChannel_1D(AOV1D var, const LightPathExpression& expr, size_t& id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = create1DBuffer();
	id			= mLPE_1D[var].size();
	mLPE_1D[var].emplace_back(std::make_pair(expr, output));
	return output;
}
inline std::shared_ptr<FrameBufferUInt32> FrameBufferContainer::requestLPEChannel_Counter(AOVCounter var, const LightPathExpression& expr, size_t& id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = createCounterBuffer();
	id			= mLPE_Counter[var].size();
	mLPE_Counter[var].emplace_back(std::make_pair(expr, output));
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestLPEChannel_3D(AOV3D var, const LightPathExpression& expr, size_t& id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = create3DBuffer();
	id			= mLPE_3D[var].size();
	mLPE_3D[var].emplace_back(std::make_pair(expr, output));
	return output;
}
inline std::shared_ptr<FrameBufferFloat> FrameBufferContainer::requestLPEChannel_Spectral(AOVSpectral var, const LightPathExpression& expr, size_t& id)
{
	PR_ASSERT(expr.isValid(), "Given expression has to be valid");
	auto output = createSpectralBuffer();
	id			= mLPE_Spectral[var].size();
	mLPE_Spectral[var].emplace_back(std::make_pair(expr, output));
	return output;
}

} // namespace PR
