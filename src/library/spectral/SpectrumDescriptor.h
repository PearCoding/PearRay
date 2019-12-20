#pragma once

#include "ColorTriplet.h"

namespace PR {
class Spectrum;
class PR_LIB SpectrumDescriptor {
public:
	virtual ~SpectrumDescriptor() = default;

	virtual size_t samples() const					 = 0;
	virtual float wavelength(size_t index) const	 = 0;
	virtual float luminousFactor(size_t index) const = 0;
	virtual float integralDelta(size_t index) const  = 0;

	virtual int tag() const = 0;

	virtual void convertSpectrum(Spectrum& dst,
								 const Spectrum& src)
		= 0;
	virtual ColorTriplet convertTriplet(
		const std::shared_ptr<SpectrumDescriptor>& other,
		const ColorTriplet& spec)
		= 0;

	static std::shared_ptr<SpectrumDescriptor> createDefault();

	static std::shared_ptr<SpectrumDescriptor> createXYZTriplet();
	static std::shared_ptr<SpectrumDescriptor> createSRGBTriplet();
	static std::shared_ptr<SpectrumDescriptor> createStandardSpectral();
};

} // namespace PR
