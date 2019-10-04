#include "ToneMapper.h"
#include "Logger.h"
#include "RGBConverter.h"
#include "Spectrum.h"
#include "XYZConverter.h"

constexpr float REINHARD_RATIO = 0.32f;
namespace PR {
ToneMapper::ToneMapper()
	: mColorMode(TCM_SRGB)
	, mGammaMode(TGM_SRGB)
	, mMapperMode(TMM_Simple_Reinhard)
{
}

void ToneMapper::map(const float* specIn, size_t specElems, size_t specElemPitch,
					 float* out, size_t rgbElems, size_t pixelCount) const
{
	for (size_t i = 0; i < pixelCount; ++i) {
		float r, g, b;

		// Map 1: Spec to RGB
		switch (mColorMode) {
		case TCM_SRGB:
			RGBConverter::convert(specElems, specElemPitch, &specIn[i], r, g, b);
			break;
		case TCM_XYZ:
			XYZConverter::convertXYZ(specElems, specElemPitch, &specIn[i], r, g, b);
			break;
		case TCM_XYZ_NORM:
			XYZConverter::convert(specElems, specElemPitch, &specIn[i], r, g);
			b = 1 - r - g;
			break;
		case TCM_LUMINANCE:
			RGBConverter::convert(specElems, specElemPitch, &specIn[i], r, g, b);
			r = RGBConverter::luminance(r, g, b);
			g = r;
			b = r;
			break;
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
	}
}
} // namespace PR
