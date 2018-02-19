#pragma once

#include "buffer/FrameBuffer.h"
#include "spectral/Spectrum.h"

#include <unordered_map>

namespace PR {
class RenderContext;
struct ShaderClosure;

class PR_LIB OutputMap {
public:
	enum Variable3D {
		V_Position = 0,
		V_Normal,
		V_NormalG,
		V_Tangent,
		V_Bitangent,
		V_View,
		V_UVW,
		V_DPDU,
		V_DPDV,
		V_DPDW,
		V_DPDT,

		V_3D_COUNT
	};

	enum Variable1D {
		V_Depth = 0,
		V_Time,
		V_Material,

		V_1D_COUNT
	};

	enum VariableCounter {
		V_ID = 0,
		V_Samples,

		V_COUNTER_COUNT
	};

	explicit OutputMap(RenderContext* renderer);
	~OutputMap();

	void init();
	void deinit();

	void clear();

	void pushFragment(const Eigen::Vector2i& p, const Spectrum& s, const ShaderClosure& sc);
	const Spectrum getFragment(const Eigen::Vector2i& p) const;

	inline void setSampleCount(const Eigen::Vector2i& p, uint64 sample)
	{
		mIntCounter[V_Samples]->setFragmentBounded(p, 0, sample);
	}

	inline uint64 getSampleCount(const Eigen::Vector2i& p) const
	{
		return mIntCounter[V_Samples]->getFragmentBounded(p, 0);
	}

	inline void incSampleCount(const Eigen::Vector2i& p)
	{
		mIntCounter[V_Samples]->setFragmentBounded(p, 0, mIntCounter[V_Samples]->getFragmentBounded(p, 0) + 1);
	}

	bool isPixelFinished(const Eigen::Vector2i& p) const;
	uint64 finishedPixelCount() const;

	inline std::shared_ptr<FrameBufferFloat> getChannel(Variable1D var) const
	{
		return mInt1D[var];
	}

	inline std::shared_ptr<FrameBufferUInt64> getChannel(VariableCounter var) const
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

	inline void registerChannel(Variable1D var, const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mInt1D[var], "Given output entry has to be set");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mInt1D[var] = output;
	}

	inline void registerChannel(VariableCounter var, const std::shared_ptr<FrameBufferUInt64>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mIntCounter[var], "Given output entry has to be set");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mIntCounter[var] = output;
	}

	inline void registerChannel(Variable3D var, const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mInt3D[var], "Given output entry has to be set");
		PR_ASSERT(output->channels() == 3, "Invalid channel count");
		mInt3D[var] = output;
	}

	inline void registerCustomChannel_1D(const std::string& str, const std::shared_ptr<FrameBufferFloat>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(output->channels() == 1, "Invalid channel count");
		mCustom1D[str] = output;
	}

	inline void registerCustomChannel_Counter(const std::string& str, const std::shared_ptr<FrameBufferUInt64>& output)
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

private:
	void setValue(Variable3D var, const Eigen::Vector2i& p, float t, const Eigen::Vector3f& val);
	RenderContext* mRenderer;

	bool mInitialized;

	std::shared_ptr<FrameBufferFloat> mSpectral;
	std::shared_ptr<FrameBufferFloat> mInt3D[V_3D_COUNT];
	std::shared_ptr<FrameBufferFloat> mInt1D[V_1D_COUNT];
	std::shared_ptr<FrameBufferUInt64> mIntCounter[V_COUNTER_COUNT];

	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustom3D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustom1D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferUInt64>> mCustomCounter;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustomSpectral;
};
} // namespace PR
