#pragma once

#include "SpectrumDescriptor.h"

namespace PR {
class PR_LIB XYZSpectrumDescriptor : public SpectrumDescriptor {
public:
	virtual ~XYZSpectrumDescriptor() = default;

	virtual size_t samples() const override;
	virtual float wavelength(size_t index) const override;
	virtual float luminousFactor(size_t index) const override;
	virtual float integralDelta(size_t index) const override;

	virtual int tag() const override;

	virtual void convertSpectrum(Spectrum& dst, const Spectrum& src) override;
	virtual SpectralBlob convertTriplet(const std::shared_ptr<SpectrumDescriptor>& other,
										const SpectralBlob& spec) override;
};

} // namespace PR
