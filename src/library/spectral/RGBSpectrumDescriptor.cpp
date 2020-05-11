#include "RGBSpectrumDescriptor.h"
#include "RGBConverter.h"
#include "Spectrum.h"
#include "SpectrumTag.h"

namespace PR {
size_t RGBSpectrumDescriptor::samples() const
{
	return 3;
}

float RGBSpectrumDescriptor::wavelength(size_t index) const
{
	static float W[3] = { 611.38f, 548.97f, 464.26f };
	return W[index];
}

float RGBSpectrumDescriptor::luminousFactor(size_t index) const
{
	static float W[3] = { PR_LUMINOSITY_RED, PR_LUMINOSITY_GREEN, PR_LUMINOSITY_BLUE };
	return W[index];
}

float RGBSpectrumDescriptor::integralDelta(size_t) const
{
	return 1;
}

int RGBSpectrumDescriptor::tag() const
{
	return ST_SRGB;
}

void RGBSpectrumDescriptor::convertSpectrum(Spectrum& dst, const Spectrum& src)
{
	PR_ASSERT(dst.descriptor()->tag() == tag(), "Expected dst to be of same tag");

	switch (src.descriptor()->tag()) {
	case ST_SRGB:
		dst = src;
		break;
	case ST_XYZ: {
		RGBConverter::fromXYZ(src[0], src[1], src[2], dst[0], dst[1], dst[2]);
		break;
	}
	default:
		PR_ASSERT(false, "Not implemented!");
		break;
	}
}

SpectralBlob RGBSpectrumDescriptor::convertTriplet(const std::shared_ptr<SpectrumDescriptor>& other,
												   const SpectralBlob& spec)
{
	switch (other->tag()) {
	case ST_SRGB:
		return spec;
	case ST_XYZ: {
		SpectralBlob newS;
		RGBConverter::fromXYZ(spec[0], spec[1], spec[2], newS[0], newS[1], newS[2]);
		return newS;
	}
	default:
		PR_ASSERT(false, "Not implemented!");
		return spec;
	}
}

} // namespace PR
