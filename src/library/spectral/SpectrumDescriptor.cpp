#include "SpectrumDescriptor.h"

namespace PR {

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createTriplet()
{
	return std::make_shared<SpectrumDescriptor>(std::vector<float>{ PR_SPECTRAL_TRIPLET_X_LAMBDA, PR_SPECTRAL_TRIPLET_Y_LAMBDA, PR_SPECTRAL_TRIPLET_Z_LAMBDA });
}

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createStandardSpectral()
{
	return std::make_shared<SpectrumDescriptor>(PR_SPECTRAL_WAVELENGTH_SAMPLES, PR_SPECTRAL_WAVELENGTH_START, PR_SPECTRAL_WAVELENGTH_END);
}
} // namespace PR
