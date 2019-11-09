#include "SpectrumDescriptor.h"

#include "RGBConverter.h"

namespace PR {
#include "xyz.inl"

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createTriplet()
{
	float xf, yf, zf;
	RGBConverter::toXYZ(PR_LUMINOSITY_RED, PR_LUMINOSITY_GREEN, PR_LUMINOSITY_BLUE, xf, yf, zf); // Standard Luminosity Factors

	return std::make_shared<SpectrumDescriptor>(
		std::vector<float>{ PR_SPECTRAL_TRIPLET_X_LAMBDA, PR_SPECTRAL_TRIPLET_Y_LAMBDA, PR_SPECTRAL_TRIPLET_Z_LAMBDA },
		std::vector<float>{ xf, yf, zf });
}

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createStandardSpectral()
{
	std::shared_ptr<SpectrumDescriptor> desc = std::make_shared<SpectrumDescriptor>(PR_SPECTRAL_WAVELENGTH_SAMPLES, PR_SPECTRAL_WAVELENGTH_START, PR_SPECTRAL_WAVELENGTH_END);

	for (uint32 i = 0; i < PR_SPECTRAL_WAVELENGTH_SAMPLES; ++i) {
		desc->setLuminousFactor(i, NM_TO_Y[i]);
	}

	return desc;
}

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createDefault()
{
	return createTriplet();
}
} // namespace PR
