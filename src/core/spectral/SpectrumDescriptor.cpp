#include "SpectrumDescriptor.h"
#include "RGBSpectrumDescriptor.h"
#include "XYZSpectrumDescriptor.h"

namespace PR {

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createXYZTriplet()
{
	return std::make_shared<XYZSpectrumDescriptor>();
}

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createSRGBTriplet()
{
	return std::make_shared<RGBSpectrumDescriptor>();
}

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createStandardSpectral()
{
	PR_ASSERT(false, "Spectral not implemented!");
	return nullptr;
}

std::shared_ptr<SpectrumDescriptor> SpectrumDescriptor::createDefault()
{
	return createSRGBTriplet();
}
} // namespace PR
