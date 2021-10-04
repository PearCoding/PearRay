#pragma once

#include "AOV.h"
#include "OutputData.h"

namespace PR {
class LocalOutputDevice;
class LightPathExpression;

/// Abstract output device
class PR_LIB_CORE OutputDevice {
public:
	virtual void clear(bool force = false)														= 0;
	virtual std::shared_ptr<LocalOutputDevice> createLocal(const Size2i& size) const			= 0;
	virtual void mergeLocal(const Point2i& p, const std::shared_ptr<LocalOutputDevice>& bucket, size_t iteration) = 0;

	virtual void enable1DChannel(AOV1D var)				= 0;
	virtual void enableCounterChannel(AOVCounter var)	= 0;
	virtual void enable3DChannel(AOV3D var)				= 0;
	virtual void enableSpectralChannel(AOVSpectral var) = 0;

	virtual void registerLPE1DChannel(AOV1D var, const LightPathExpression& expr, uint32 id)			 = 0;
	virtual void registerLPECounterChannel(AOVCounter var, const LightPathExpression& expr, uint32 id)	 = 0;
	virtual void registerLPE3DChannel(AOV3D var, const LightPathExpression& expr, uint32 id)			 = 0;
	virtual void registerLPESpectralChannel(AOVSpectral var, const LightPathExpression& expr, uint32 id) = 0;

	virtual void registerCustom1DChannel(const std::string& str, uint32 id)		  = 0;
	virtual void registerCustomCounterChannel(const std::string& str, uint32 id)  = 0;
	virtual void registerCustom3DChannel(const std::string& str, uint32 id)		  = 0;
	virtual void registerCustomSpectralChannel(const std::string& str, uint32 id) = 0;

	virtual const char* type() const = 0;
};
} // namespace PR