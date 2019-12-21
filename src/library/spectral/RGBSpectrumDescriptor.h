#pragma once

#include "SpectrumDescriptor.h"

namespace PR {
// SRGB D65
class PR_LIB RGBSpectrumDescriptor : public SpectrumDescriptor,
									 std::enable_shared_from_this<RGBSpectrumDescriptor> {
public:
	virtual ~RGBSpectrumDescriptor() = default;

	virtual size_t samples() const override;
	virtual float wavelength(size_t index) const override;
	virtual float luminousFactor(size_t index) const override;
	virtual float integralDelta(size_t index) const override;

	virtual int tag() const override;

	virtual void convertSpectrum(Spectrum& dst, const Spectrum& src) override;
	virtual ColorTriplet convertTriplet(const std::shared_ptr<SpectrumDescriptor>& other,
										const ColorTriplet& spec) override;
};

} // namespace PR
