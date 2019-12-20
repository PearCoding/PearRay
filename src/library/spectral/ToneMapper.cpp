#include "ToneMapper.h"
#include "Logger.h"
#include "RGBConverter.h"
#include "Spectrum.h"
#include "SpectrumDescriptor.h"

constexpr float REINHARD_RATIO = 0.32f;
namespace PR {
ToneMapper::ToneMapper()
	: mColorMode(TCM_SRGB)
	, mGammaMode(TGM_SRGB)
	, mMapperMode(TMM_Simple_Reinhard)
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

	// Map 2: Tone Mapping
	if (mMapperMode != TMM_None)
		mapOnlyMapper(out, out, rgbElems, pixelCount);

	// Map 3: Gamma Correction
	if (mGammaMode != TGM_None) {
		for (size_t i = 0; i < pixelCount; ++i) {
			float r, g, b;
			r = out[i * rgbElems];
			g = out[i * rgbElems + 1];
			b = out[i * rgbElems + 2];

			RGBConverter::gamma(r, g, b);

			out[i * rgbElems]	 = r;
			out[i * rgbElems + 1] = g;
			out[i * rgbElems + 2] = b;
		}
	}
}

void ToneMapper::mapOnlyMapper(const float* rgbIn, float* rgbOut,
							   size_t rgbElems, size_t pixelCount) const
{
	if (mMapperMode == TMM_None) { // Fast approach
		std::memcpy(rgbOut, rgbIn, sizeof(float) * rgbElems * pixelCount);
		return;
	}

	float invMax = 0.0f;
	if (mMapperMode == TMM_Normalized) {
		for (size_t i = 0; i < pixelCount; ++i) {
			float r, g, b;
			r = rgbIn[i * rgbElems];
			g = rgbIn[i * rgbElems + 1];
			b = rgbIn[i * rgbElems + 2];

			invMax = std::max(r * r + g * g + b * b, invMax);
		}
		invMax = std::sqrt(invMax);

		if (invMax <= PR_EPSILON)
			return;

		invMax = 1.0f / invMax;
	}

	for (size_t i = 0; i < pixelCount; ++i) {
		float r, g, b;
		r = rgbIn[i * rgbElems];
		g = rgbIn[i * rgbElems + 1];
		b = rgbIn[i * rgbElems + 2];

		switch (mMapperMode) {
		case TMM_None:
			break;
		case TMM_Simple_Reinhard: {
			float Ld = 1 / (1 + RGBConverter::luminance(r, g, b) * REINHARD_RATIO);
			r *= Ld;
			g *= Ld;
			b *= Ld;
		} break;
		case TMM_Normalized:
			r *= invMax;
			g *= invMax;
			b *= invMax;
			break;
		case TMM_Clamp:
			r = std::max(std::abs(r), 0.0f);
			g = std::max(std::abs(g), 0.0f);
			b = std::max(std::abs(b), 0.0f);
			break;
		case TMM_Abs:
			r = std::abs(r);
			g = std::abs(g);
			b = std::abs(b);
			break;
		case TMM_Positive:
			r = std::max(r, 0.0f);
			g = std::max(g, 0.0f);
			b = std::max(b, 0.0f);
			break;
		case TMM_Negative:
			r = std::max(-r, 0.0f);
			g = std::max(-g, 0.0f);
			b = std::max(-b, 0.0f);
			break;
		case TMM_Spherical:
			r = 0.5f + 0.5f * std::atan2(b, r) * PR_1_PI;
			g = 0.5f - std::asin(-g) * PR_1_PI;
			b = 0;
			break;
		}

		rgbOut[i * rgbElems]	 = r;
		rgbOut[i * rgbElems + 1] = g;
		rgbOut[i * rgbElems + 2] = b;
		if (rgbElems == 4)
			rgbOut[i * rgbElems + 3] = 1;
	}
}
} // namespace PR
