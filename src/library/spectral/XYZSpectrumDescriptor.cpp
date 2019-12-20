#include "XYZSpectrumDescriptor.h"
#include "RGBConverter.h"
#include "Spectrum.h"
#include "SpectrumTag.h"

namespace PR {
size_t XYZSpectrumDescriptor::samples() const
{
	return 3;
}

float XYZSpectrumDescriptor::wavelength(size_t index) const
{
	static float W[3] = { -500.2f, 522.1f, 477.1f };
	return W[index];
}

float XYZSpectrumDescriptor::luminousFactor(size_t index) const
{
	return index == 1 ? 1.0f : 0.0f;
}

float XYZSpectrumDescriptor::integralDelta(size_t) const
{
	return 1;
}

int XYZSpectrumDescriptor::tag() const
{
	return ST_XYZ;
}

void XYZSpectrumDescriptor::convertSpectrum(Spectrum& dst, const Spectrum& src)
{
	PR_ASSERT(dst.descriptor()->tag() == tag(), "Expected dst to be of same tag");

	switch (src.descriptor()->tag()) {
	case ST_XYZ:
		dst = src;
		break;
	case ST_SRGB: {
		RGBConverter::toXYZ(src[0], src[1], src[2], dst[0], dst[1], dst[2]);
		break;
	}
	default:
		PR_ASSERT(false, "Not implemented!");
		break;
	}
}

// TODO: This is missing important information
ColorTriplet XYZSpectrumDescriptor::convertTriplet(const std::shared_ptr<SpectrumDescriptor>& other,
												   const ColorTriplet& spec)
{
	switch (other->tag()) {
	case ST_XYZ:
		return spec;
	case ST_SRGB: {
		ColorTriplet newS;
		RGBConverter::toXYZ(spec[0], spec[1], spec[2], newS[0], newS[1], newS[2]);
		return newS;
	}
	default:
		PR_ASSERT(false, "Not implemented!");
		return spec;
	}
}

} // namespace PR
