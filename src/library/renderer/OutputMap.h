#pragma once

#include "buffer/FrameBuffer.h"

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

	inline Spectrum getFragment(const Eigen::Vector2i& p) const
	{
		return mSpectral->getFragmentBounded(p);
	}

	inline void setSampleCount(const Eigen::Vector2i& p, uint64 sample)
	{
		mIntCounter[V_Samples]->setFragmentBounded(p, sample);
	}

	inline uint64 getSampleCount(const Eigen::Vector2i& p) const
	{
		return mIntCounter[V_Samples]->getFragmentBounded(p);
	}

	inline void incSampleCount(const Eigen::Vector2i& p)
	{
		mIntCounter[V_Samples]->setFragmentBounded(p, mIntCounter[V_Samples]->getFragmentBounded(p) + 1);
	}

	bool isPixelFinished(const Eigen::Vector2i& p) const;
	uint64 finishedPixelCount() const;

	inline const std::shared_ptr<FrameBuffer1D>& getChannel(Variable1D var) const
	{
		return mInt1D[var];
	}

	inline const std::shared_ptr<FrameBufferCounter>& getChannel(VariableCounter var) const
	{
		return mIntCounter[var];
	}

	inline const std::shared_ptr<FrameBuffer3D>& getChannel(Variable3D var) const
	{
		return mInt3D[var];
	}

	inline const std::shared_ptr<FrameBufferSpectrum>& getSpectralChannel() const
	{
		return mSpectral;
	}

	inline void registerChannel(Variable1D var, const std::shared_ptr<FrameBuffer1D>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mInt1D[var], "Given output entry has to be set");
		mInt1D[var] = output;
	}

	inline void registerChannel(VariableCounter var, const std::shared_ptr<FrameBufferCounter>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mIntCounter[var], "Given output entry has to be set");
		mIntCounter[var] = output;
	}

	inline void registerChannel(Variable3D var, const std::shared_ptr<FrameBuffer3D>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!mInt3D[var], "Given output entry has to be set");
		mInt3D[var] = output;
	}

	inline void registerCustomChannel(const std::string& str, const std::shared_ptr<FrameBuffer1D>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		mCustom1D[str] = output;
	}

	inline void registerCustomChannel(const std::string& str, const std::shared_ptr<FrameBufferCounter>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		mCustomCounter[str] = output;
	}

	inline void registerCustomChannel(const std::string& str, const std::shared_ptr<FrameBuffer3D>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		mCustom3D[str] = output;
	}

	inline void registerCustomChannel(const std::string& str, const std::shared_ptr<FrameBufferSpectrum>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		mCustomSpectral[str] = output;
	}

private:
	RenderContext* mRenderer;

	bool mInitialized;
	
	std::shared_ptr<FrameBufferSpectrum> mSpectral;
	std::shared_ptr<FrameBuffer3D> mInt3D[V_3D_COUNT];
	std::shared_ptr<FrameBuffer1D> mInt1D[V_1D_COUNT];
	std::shared_ptr<FrameBufferCounter> mIntCounter[V_COUNTER_COUNT];

	std::unordered_map<std::string, std::shared_ptr<FrameBuffer3D>> mCustom3D;
	std::unordered_map<std::string, std::shared_ptr<FrameBuffer1D>> mCustom1D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferCounter>> mCustomCounter;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferSpectrum>> mCustomSpectral;
};
}
