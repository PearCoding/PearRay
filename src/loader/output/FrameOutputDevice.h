#pragma once

#include "FrameContainer.h"
#include "output/OutputDevice.h"

#include <mutex>

namespace PR {
class IFilter;
class PR_LIB_CORE FrameOutputDevice : public OutputDevice {
public:
	explicit FrameOutputDevice(const std::shared_ptr<IFilter>& filter,
							   const Size2i& size, Size1i specChannels, bool monotonic);
	virtual ~FrameOutputDevice();

	inline FrameContainer& data() { return mData; }
	inline const FrameContainer& data() const { return mData; }

	// Mandatory interface

	void clear(bool force = false) override;
	std::shared_ptr<LocalOutputDevice> createLocal(const Size2i& size) const override;
	void mergeLocal(const Point2i& p, const std::shared_ptr<LocalOutputDevice>& bucket, size_t iteration) override;

	void enable1DChannel(AOV1D var) override;
	void enableCounterChannel(AOVCounter var) override;
	void enable3DChannel(AOV3D var) override;
	void enableSpectralChannel(AOVSpectral var) override;

	void registerLPE1DChannel(AOV1D var, const LightPathExpression& expr, uint32 id) override;
	void registerLPECounterChannel(AOVCounter var, const LightPathExpression& expr, uint32 id) override;
	void registerLPE3DChannel(AOV3D var, const LightPathExpression& expr, uint32 id) override;
	void registerLPESpectralChannel(AOVSpectral var, const LightPathExpression& expr, uint32 id) override;

	void registerCustom1DChannel(const std::string& str, uint32 id) override;
	void registerCustomCounterChannel(const std::string& str, uint32 id) override;
	void registerCustom3DChannel(const std::string& str, uint32 id) override;
	void registerCustomSpectralChannel(const std::string& str, uint32 id) override;

	const char* type() const override { return "pr_frameoutputdevice"; };

private:
	const std::shared_ptr<IFilter> mFilter;
	const bool mMonotonic;

	FrameContainer mData;
	std::mutex mMergeMutex;
};
} // namespace PR
