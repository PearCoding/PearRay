#include "ToneMapper.h"
#include "Logger.h"
#include "RGBConverter.h"
#include "Spectrum.h"
#include "SpectrumDescriptor.h"

namespace PR {
ToneMapper::ToneMapper()
	: mColorMode(TCM_SRGB)
{
}

void ToneMapper::map(const std::shared_ptr<SpectrumDescriptor>& desc,
					 const float* specIn,
					 float* out, size_t rgbElems, size_t pixelCount) const
{
	std::shared_ptr<SpectrumDescriptor> newDesc;
	switch (mColorMode) {
	case TCM_SRGB:
		newDesc = SpectrumDescriptor::createSRGBTriplet();
		break;
	case TCM_XYZ:
	case TCM_XYZ_NORM:
	case TCM_LUMINANCE:
		newDesc = SpectrumDescriptor::createXYZTriplet();
		break;
	}

	size_t specElems = desc->samples();
	for (size_t i = 0; i < pixelCount; ++i) {
		float r, g, b;

		// Map 1: Spec to Triplet
		switch (mColorMode) {
		default:
		case TCM_SRGB: {
			Spectrum newS(newDesc);
			newDesc->convertSpectrum(newS,
									 Spectrum(desc, 0, specElems, const_cast<float*>(&specIn[i * specElems])));
			r = newS[0];
			g = newS[1];
			b = newS[2];
		} break;
		case TCM_XYZ: {
			Spectrum newS(newDesc);
			newDesc->convertSpectrum(newS,
									 Spectrum(desc, 0, specElems, const_cast<float*>(&specIn[i * specElems])));
			r = newS[0];
			g = newS[1];
			b = newS[2];
		} break;
		case TCM_XYZ_NORM: {
			Spectrum newS(newDesc);
			newDesc->convertSpectrum(newS,
									 Spectrum(desc, 0, specElems, const_cast<float*>(&specIn[i * specElems])));
			r		= newS[0];
			g		= newS[1];
			b		= newS[2];
			float n = r + g + b;
			r /= n;
			g /= n;
			b /= n;
		} break;
		case TCM_LUMINANCE: {
			Spectrum newS(newDesc);
			newDesc->convertSpectrum(newS,
									 Spectrum(desc, 0, specElems, const_cast<float*>(&specIn[i * specElems])));
			r = newS.luminousFlux();
			g = r;
			b = r;
		} break;
		}

		out[i * rgbElems]	 = r;
		out[i * rgbElems + 1] = g;
		out[i * rgbElems + 2] = b;
	}
}
} // namespace PR
