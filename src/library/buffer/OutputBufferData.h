#pragma once

#include "buffer/AOV.h"
#include "buffer/FrameBuffer.h"
#include "path/LightPathExpression.h"

#include <unordered_map>

namespace PR {
class ShadingPoint;
class LightPath;

/* Three types of channel provider:
 * Internal -> Can be requested by the integrator/user for common shading variables
 * Custom   -> Can be requested by the integrator for buffer wide data
 * LPE      -> Can be requested by the (integrator)/user for custom composition
 *
 * Four types of buffer type:
 * Spectral -> XYZ or other type of spectral data
 * 3D       -> 3D Vector data
 * 1D       -> 1D float data
 * Counter  -> 1D uint32 data
 */
class PR_LIB OutputBufferData {
	friend class OutputBuffer;
	friend class OutputBufferBucket;

public:
	OutputBufferData(const Size2i& size, Size1i specChannels);
	~OutputBufferData();

	void clear(bool force = false);

	// Internal
	inline bool hasInternalChannel_1D(AOV1D var) const;
	inline bool hasInternalChannel_Counter(AOVCounter var) const;
	inline bool hasInternalChannel_3D(AOV3D var) const;
	inline bool hasInternalChannel_Spectral() const;

	inline std::shared_ptr<FrameBufferFloat> getInternalChannel_1D(AOV1D var) const;
	inline std::shared_ptr<FrameBufferUInt32> getInternalChannel_Counter(AOVCounter var) const;
	inline std::shared_ptr<FrameBufferFloat> getInternalChannel_3D(AOV3D var) const;
	inline std::shared_ptr<FrameBufferFloat> getInternalChannel_Spectral() const;

	inline std::shared_ptr<FrameBufferFloat> requestInternalChannel_1D(AOV1D var);
	inline std::shared_ptr<FrameBufferUInt32> requestInternalChannel_Counter(AOVCounter var);
	inline std::shared_ptr<FrameBufferFloat> requestInternalChannel_3D(AOV3D var);
	inline std::shared_ptr<FrameBufferFloat> requestInternalChannel_Spectral();

	// Custom
	inline bool hasCustomChannel_1D(const std::string& str) const;
	inline bool hasCustomChannel_Counter(const std::string& str) const;
	inline bool hasCustomChannel_3D(const std::string& str) const;
	inline bool hasCustomChannel_Spectral(const std::string& str) const;

	inline std::shared_ptr<FrameBufferFloat> getCustomChannel_1D(const std::string& str) const;
	inline std::shared_ptr<FrameBufferUInt32> getCustomChannel_Counter(const std::string& str) const;
	inline std::shared_ptr<FrameBufferFloat> getCustomChannel_3D(const std::string& str) const;
	inline std::shared_ptr<FrameBufferFloat> getCustomChannel_Spectral(const std::string& str) const;

	inline std::shared_ptr<FrameBufferFloat> requestCustomChannel_1D(const std::string& str);
	inline std::shared_ptr<FrameBufferUInt32> requestCustomChannel_Counter(const std::string& str);
	inline std::shared_ptr<FrameBufferFloat> requestCustomChannel_3D(const std::string& str);
	inline std::shared_ptr<FrameBufferFloat> requestCustomChannel_Spectral(const std::string& str);

	// LPE
	inline size_t getLPEChannelCount_1D(AOV1D var) const;
	inline size_t getLPEChannelCount_Counter(AOVCounter var) const;
	inline size_t getLPEChannelCount_3D(AOV3D var) const;
	inline size_t getLPEChannelCount_Spectral() const;

	inline std::shared_ptr<FrameBufferFloat> getLPEChannel_1D(AOV1D var, size_t i) const;
	inline std::shared_ptr<FrameBufferUInt32> getLPEChannel_Counter(AOVCounter var, size_t i) const;
	inline std::shared_ptr<FrameBufferFloat> getLPEChannel_3D(AOV3D var, size_t i) const;
	inline std::shared_ptr<FrameBufferFloat> getLPEChannel_Spectral(size_t i) const;

	inline std::shared_ptr<FrameBufferFloat> requestLPEChannel_1D(AOV1D var, const LightPathExpression& expr, size_t& id);
	inline std::shared_ptr<FrameBufferUInt32> requestLPEChannel_Counter(AOVCounter var, const LightPathExpression& expr, size_t& id);
	inline std::shared_ptr<FrameBufferFloat> requestLPEChannel_3D(AOV3D var, const LightPathExpression& expr, size_t& id);
	inline std::shared_ptr<FrameBufferFloat> requestLPEChannel_Spectral(const LightPathExpression& expr, size_t& id);

private:
	std::shared_ptr<FrameBufferFloat> createSpectralBuffer() const;
	std::shared_ptr<FrameBufferFloat> create3DBuffer() const;
	std::shared_ptr<FrameBufferFloat> create1DBuffer() const;
	std::shared_ptr<FrameBufferUInt32> createCounterBuffer() const;

	std::shared_ptr<FrameBufferFloat> mSpectral;
	std::shared_ptr<FrameBufferFloat> mInt3D[AOV_3D_COUNT];
	std::shared_ptr<FrameBufferFloat> mInt1D[AOV_1D_COUNT];
	std::shared_ptr<FrameBufferUInt32> mIntCounter[AOV_COUNTER_COUNT];

	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustom3D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustom1D;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferUInt32>> mCustomCounter;
	std::unordered_map<std::string, std::shared_ptr<FrameBufferFloat>> mCustomSpectral;

	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferFloat>>> mLPE_Spectral;
	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferFloat>>> mLPE_3D[AOV_3D_COUNT];
	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferFloat>>> mLPE_1D[AOV_1D_COUNT];
	std::vector<std::pair<LightPathExpression, std::shared_ptr<FrameBufferUInt32>>> mLPE_Counter[AOV_COUNTER_COUNT];
};
} // namespace PR

#include "OutputBufferData.inl"