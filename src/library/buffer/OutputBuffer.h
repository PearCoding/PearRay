#pragma once

#include "buffer/FrameBuffer.h"
#include "path/LightPathExpression.h"

#include <unordered_map>

namespace PR {
class RenderContext;
struct ShadingPoint;
class LightPath;

class PR_LIB OutputBuffer {
public:
	enum Variable3D {
		V_Position = 0,
		V_Normal,
		V_NormalG,
		V_Tangent,
		V_Bitangent,
		V_View,
		V_UVW,
		V_DPDT,

		V_3D_COUNT
	};

	enum Variable1D {
		V_EntityID = 0,
		V_MaterialID,
		V_EmissionID,
		V_DisplaceID,
		V_Depth,
		V_Time,

		V_1D_COUNT
	};

	enum VariableCounter {
		V_Samples = 0,
		V_Feedback,

		V_COUNTER_COUNT
	};

	explicit OutputBuffer(RenderContext* renderer);
	~OutputBuffer();

	void clear();

	void pushFragment(uint32 pixelIndex, const ShadingPoint& pt, const LightPath& path);
	void pushBackgroundFragment(uint32 pixelIndex, uint32 channel);
	void pushFeedbackFragment(uint32 pixelIndex, uint32 channel, uint32 feedback);

	inline std::shared_ptr<FrameBufferFloat> getChannel(Variable1D var) const
	{
		return mInt1D[var];
	}

	inline std::shared_ptr<FrameBufferUInt32> getChannel(VariableCounter var) const
	{
		return mIntCounter[var];
	}

	inline std::shared_ptr<FrameBufferFloat> getChannel(Variable3D var) const
	{
		return mInt3D[var];
	}

	inline std::shared_ptr<FrameBufferFloat> getSpectralChannel() const
	{
		return mSpectral;
	}

	// LPE
	inline std::shared_ptr<FrameBufferFloat> getChannel(Variable1D var, size_t i) const
	{
		return mLPE_1D[var].at(i).second;
	}

	inline std::shared_ptr<FrameBufferUInt32> getChannel(VariableCounter var, size_t i) const
	{
		return mLPE_Counter[var].at(i).second;
	}

	inline std::shared_ptr<FrameBufferFloat> getChannel(Variable3D var, size_t i) const
	{
		return mLPE_3D[var].at(i).second;
	}

	inline std::shared_ptr<FrameBufferFloat> getSpectralChannel(size_t i) const
	{
		return mLPE_Spectral.at(i).second;
	}

	// Register
	inline void registerChannel(Variable1D var,
								const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mInt1D[var], "Given output entry has to be set");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mInt1D[var] = output;
	}

	inline void registerChannel(VariableCounter var,
								const std::shared_ptr<FrameBufferUInt32>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mIntCounter[var], "Given output entry has to be set");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mIntCounter[var] = output;
	}

	inline void registerChannel(Variable3D var,
								const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mInt3D[var], "Given output entry has to be set");
		PR_ASSERT(output->channels() == 3, "Invalid channel count");
		mInt3D[var] = output;
	}

	// Integrator
	inline void registerCustomChannel_1D(const std::string& str, const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mCustom1D[str] = output;
	}

	inline void registerCustomChannel_Counter(const std::string& str, const std::shared_ptr<FrameBufferUInt32>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mCustomCounter[str] = output;
	}

	inline void registerCustomChannel_3D(const std::string& str, const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 3, "Invalid channel count");
		mCustom3D[str] = output;
	}

	inline void registerCustomChannel_Spectral(const std::string& str, const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		mCustomSpectral[str] = output;
	}

	// User
	inline void registerLPEChannel(Variable1D var,
								   const LightPathExpression& expr,
								   const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(expr.isValid(), "Given expression has to be valid");
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mLPE_1D[var].emplace_back(std::make_pair(expr, output));
	}

	inline void registerLPEChannel(VariableCounter var,
								   const LightPathExpression& expr,
								   const std::shared_ptr<FrameBufferUInt32>& output)
	{
		PR_ASSERT(expr.isValid(), "Given expression has to be valid");
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mLPE_Counter[var].emplace_back(std::make_pair(expr, output));
	}

	inline void registerLPEChannel(Variable3D var,
								   const LightPathExpression& expr,
								   const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(expr.isValid(), "Given expression has to be valid");
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 3, "Invalid channel count");
		mLPE_3D[var].emplace_back(std::make_pair(expr, output));
	}

	inline void registerLPEChannel(const LightPathExpression& expr,
								   const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(expr.isValid(), "Given expression has to be valid");
		PR_ASSERT(output, "Given output has to be valid");
		mLPE_Spectral.emplace_back(std::make_pair(expr, output));
	}

private:
	RenderContext* mRenderer;

	std::shared_ptr<FrameBufferFloat> mSpectral;
	std::shared_ptr<FrameBufferFloat> mInt3D[V_3D_COUNT];
	std::shared_ptr<FrameBufferFloat> mInt1D[V_1D_COUNT];
	std::shared_ptr<FrameBufferUInt32> mIntCounter[V_COUNTER_COUNT];

	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustom3D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustom1D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferUInt32>> mCustomCounter;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustomSpectral;

	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferFloat>>> mLPE_Spectral;
	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferFloat>>> mLPE_3D[V_3D_COUNT];
	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferFloat>>> mLPE_1D[V_1D_COUNT];
	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferUInt32>>> mLPE_Counter[V_COUNTER_COUNT];
};
} // namespace PR
