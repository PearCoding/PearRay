#pragma once

#include "AOV.h"
#include "PR_Config.h"

#include <functional>

namespace PR {
class OutputDevice;
class LocalOutputSystem;
class LightPathExpression;

class RenderTile;
struct OutputSpectralEntry;
struct OutputFeedbackEntry;

using OutputSpectralCallback = std::function<void(const RenderTile*, const OutputSpectralEntry*, size_t)>;
using OutputFeedbackCallback = std::function<void(const RenderTile*, const OutputFeedbackEntry*, size_t)>;

/// Core Output system
/// Three types of channel provider:
/// Internal -> Can be requested by the integrator/user for common shading variables
/// Custom   -> Can be requested by the integrator for buffer wide data
/// LPE      -> Can be requested by the (integrator)/user for custom composition
///
/// Four types of buffer type:
/// Spectral -> XYZ or other type of spectral data
/// 3D       -> 3D Vector data
/// 1D       -> 1D float data
/// Counter  -> 1D uint32 data

class PR_LIB_CORE OutputSystem {
public:
	using CustomNameMap = std::unordered_map<std::string, uint32>;

	explicit OutputSystem(const Size2i& size);
	~OutputSystem();

	inline const Size2i& size() const { return mSize; }

	/// Add an abstract output device to the container. This should be called before all other methods
	void addOutputDevice(const std::shared_ptr<OutputDevice>& device);
	inline const std::vector<std::shared_ptr<OutputDevice>>& outputDevices() const { return mOutputDevices; }

	void clear(bool force = false);

	std::shared_ptr<LocalOutputSystem> createLocal(const RenderTile* tile, const Size2i& size) const;
	void mergeLocal(const Point2i& p, const std::shared_ptr<LocalOutputSystem>& local);

	/// It triggers an undefined behavior if the following methods are called after the renderer started

	void enableVarianceEstimation();
	void enable1DChannel(AOV1D var);
	void enableCounterChannel(AOVCounter var);
	void enable3DChannel(AOV3D var);
	void enableSpectralChannel(AOVSpectral var);

	uint32 registerLPE1DChannel(AOV1D var, const LightPathExpression& expr);
	uint32 registerLPECounterChannel(AOVCounter var, const LightPathExpression& expr);
	uint32 registerLPE3DChannel(AOV3D var, const LightPathExpression& expr);
	uint32 registerLPESpectralChannel(AOVSpectral var, const LightPathExpression& expr);

	uint32 registerCustom1DChannel(const std::string& str);
	uint32 registerCustomCounterChannel(const std::string& str);
	uint32 registerCustom3DChannel(const std::string& str);
	uint32 registerCustomSpectralChannel(const std::string& str);

	inline bool hasCustom1DChannel(const std::string& str) { return mCustom1DChannelMap.count(str) > 0; }
	inline bool hasCustomCounterChannel(const std::string& str) { return mCustomCounterChannelMap.count(str) > 0; }
	inline bool hasCustom3DChannel(const std::string& str) { return mCustom3DChannelMap.count(str) > 0; }
	inline bool hasCustomSpectralChannel(const std::string& str) { return mCustomSpectralChannelMap.count(str) > 0; }

	inline const CustomNameMap& custom1DChannels() const { return mCustom1DChannelMap; }
	inline const CustomNameMap& custom3DChannels() const { return mCustom3DChannelMap; }
	inline const CustomNameMap& customCounterChannels() const { return mCustomCounterChannelMap; }
	inline const CustomNameMap& customSpectralChannels() const { return mCustomSpectralChannelMap; }

	inline void registerSpectralCallback(const OutputSpectralCallback& callback) { mSpectralCallbacks.push_back(callback); }
	inline void registerFeedbackCallback(const OutputFeedbackCallback& callback) { mFeedbackCallbacks.push_back(callback); }

	inline const std::vector<OutputSpectralCallback>& spectralCallbacks() const { return mSpectralCallbacks; }
	inline const std::vector<OutputFeedbackCallback>& feedbackCallbacks() const { return mFeedbackCallbacks; }

private:
	const Size2i mSize;
	std::vector<std::shared_ptr<OutputDevice>> mOutputDevices;
	CustomNameMap mCustom1DChannelMap;
	CustomNameMap mCustomCounterChannelMap;
	CustomNameMap mCustom3DChannelMap;
	CustomNameMap mCustomSpectralChannelMap;

	uint32 mLPE1DCount;
	uint32 mLPE3DCount;
	uint32 mLPECounterCount;
	uint32 mLPESpectralCount;

	std::vector<OutputSpectralCallback> mSpectralCallbacks;
	std::vector<OutputFeedbackCallback> mFeedbackCallbacks;
};
} // namespace PR
