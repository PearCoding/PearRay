#pragma once

#include <algorithm>
#include <cmath>

#include "PR_Config.h"

namespace PR {
namespace UI {
// Mapping mostly based on http://www.brucelindbloom.com/
enum class ColorFormat {
	XYZ = 0,
	SRGB,
	AdobeRGB,
	LAB,
	LUV
};

enum class GammaType {
	None = 0,
	SRGB,
	ReverseSRGB,
	AdobeRGB,
	ReverseAdobeRGB,
	Custom
};

// TODO: Maybe make it adaptable?
/*constexpr float REF_WHITE_X = 1 / 3.0f;
constexpr float REF_WHITE_Y = 1 / 3.0f;
constexpr float REF_WHITE_Z = 1 / 3.0f;*/

// D65 2deg
constexpr float REF_WHITE_X = 0.31271f;
constexpr float REF_WHITE_Y = 0.32902f;
constexpr float REF_WHITE_Z = 0.35827f;

constexpr float CIE_ETA	  = 0.008856f;
constexpr float CIE_KAPPA = 903.3f;

enum class ToneMappingMode {
	None = 0,
	Reinhard,
	ModifiedReinhard,
	ACES
};

/* Pipeline:
 (1) Range
 (2) Src to XYZ
 (3) Exposure + Offset
 (4) Tone Mapping
 (5) XYZ to Dst
 (6) Gamma
*/
class PR_LIB_UI ImagePipeline {
public:
	inline ImagePipeline()
		: mExposureFactor(1)
		, mOffset(0)
		, mMinRange(1) // Disabled if min > max
		, mMaxRange(0)
		, mTripletFormatSrc(ColorFormat::XYZ)
		, mTripletFormatDst(ColorFormat::XYZ)
		, mToneMapping(ToneMappingMode::None)
		, mGammaType(GammaType::SRGB)
		, mGamma(-1)
	{
	}
	~ImagePipeline() = default;

	inline void setExposure(float exposure) { mExposureFactor = std::pow(2, exposure); }
	inline void setOffset(float offset) { mOffset = offset; }
	inline void setMaxRange(float f) { mMaxRange = f; }
	inline void setMinRange(float f) { mMinRange = f; }
	inline void setToneMapping(ToneMappingMode tm) { mToneMapping = tm; }

	inline void setTripletFormat(ColorFormat formatFrom, ColorFormat formatTo)
	{
		mTripletFormatSrc = formatFrom;
		mTripletFormatDst = formatTo;
	}

	inline void setGamma(float gamma) { mGamma = gamma; }
	inline void setGammaType(GammaType type) { mGammaType = type; }

	inline void mapTripletUnbounded(float x, float y, float z, float& r, float& g, float& b) const
	{
		// (1)
		x = mapRange(x);
		y = mapRange(y);
		z = mapRange(z);

		// (2)
		formatTripletToXYZ(mTripletFormatSrc, x, y, z, r, g, b);

		// (3)
		x = applyExposureOffset(r);
		y = applyExposureOffset(g);
		z = applyExposureOffset(b);

		// (4)
		const float lum = mapTone(y);
		const float f	= (std::abs(y) <= PR_EPSILON) ? 1.0f : lum / y;
		x *= f;
		y *= f;
		z *= f;

		// (5)
		formatTripletFromXYZ(mTripletFormatDst, x, y, z, r, g, b);

		// (6)
		r = applyGamma(r);
		g = applyGamma(g);
		b = applyGamma(b);
	}

	inline void mapTriplet(float x, float y, float z, float& r, float& g, float& b) const
	{
		mapTripletUnbounded(x, y, z, r, g, b);
		// (7)
		r = clamp01(r);
		g = clamp01(g);
		b = clamp01(b);
	}

private:
	void formatTripletToXYZ(ColorFormat format,
							float r, float g, float b,
							float& x, float& y, float& z) const
	{
		switch (format) {
		default:
		case ColorFormat::XYZ:
			x = r;
			y = g;
			z = b;
			break;
		case ColorFormat::SRGB:
			x = +4.123908e-01f * r + 3.575843e-01f * g + 1.804808e-01f * b;
			y = +2.126390e-01f * r + 7.151687e-01f * g + 7.219232e-02f * b;
			z = +1.933082e-02f * r + 1.191948e-01f * g + 9.505322e-01f * b;
			break;
		case ColorFormat::AdobeRGB:
			x = 0.5767309 * r + 0.1855540 * g + 0.1881852 * b;
			y = 0.2973769 * r + 0.6273491 * g + 0.0752741 * b;
			z = 0.0270343 * r + 0.0706872 * g + 0.9911085 * b;
			break;
		case ColorFormat::LAB: {
			// Maybe from 0,1 to common range?
			const float L = r;
			const float A = g;
			const float B = b;

			const float fy = (L + 16) / 116;
			const float fx = A / 500 + fy;
			const float fz = fy - B / 200;

			const float fx3 = fx * fx * fx;
			const float fz3 = fz * fz * fz;

			const float xr = fx3 <= CIE_ETA ? (116 * fx - 16) / CIE_KAPPA : fx3;
			const float zr = fz3 <= CIE_ETA ? (116 * fz - 16) / CIE_KAPPA : fz3;
			const float yr = L <= CIE_ETA * CIE_KAPPA ? L / CIE_KAPPA : std::pow((L + 16) / 116, 3);

			x = REF_WHITE_X * xr;
			y = REF_WHITE_Y * yr;
			z = REF_WHITE_Z * zr;
		} break;
		case ColorFormat::LUV: {
			// Maybe from 0,1 to common range?
			const float L = r;
			const float U = g;
			const float V = b;

			const float ur = 4 * REF_WHITE_X / (REF_WHITE_X + 15 * REF_WHITE_Y + 3 * REF_WHITE_Z);
			const float vr = 9 * REF_WHITE_Y / (REF_WHITE_X + 15 * REF_WHITE_Y + 3 * REF_WHITE_Z);

			const float y = L <= CIE_ETA * CIE_KAPPA ? L / CIE_KAPPA : std::pow((L + 16) / 116, 3);

			const float A = (52 * L / (U + 13 * L * ur) - 1) / 3;
			const float B = -5 * y;
			const float C = -1 / 3.0f;
			const float D = y * (39 * L / (V + 13 * L * vr) * 5);

			x = (D - B) / (A - C);
			z = x * A + B;
		} break;
		}
	}

	void formatTripletFromXYZ(ColorFormat format,
							  float x, float y, float z,
							  float& r, float& g, float& b) const
	{
		switch (format) {
		default:
		case ColorFormat::XYZ:
			r = x;
			g = y;
			b = z;
			break;
		case ColorFormat::SRGB:
			r = +3.240970e+00f * x - 1.537383e+00f * y - 4.986108e-01f * z;
			g = -9.692436e-01f * x + 1.875968e+00f * y + 4.155506e-02f * z;
			b = +5.563008e-02f * x - 2.039770e-01f * y + 1.056972e+00f * z;
			break;
		case ColorFormat::AdobeRGB:
			r = +2.0413690 * x - 0.5649464 * y - 0.3446944 * z;
			g = -0.9692660 * x + 1.8760108 * y + 0.0415560 * z;
			b = +0.0134474 * x - 0.1183897 * y + 1.0154096 * z;
			break;
		case ColorFormat::LAB: {
			const float xr = x / REF_WHITE_X;
			const float yr = y / REF_WHITE_Y;
			const float zr = z / REF_WHITE_Z;

			const float fx = xr <= CIE_ETA ? (CIE_KAPPA * xr + 16) / 116.0f : std::cbrt(xr);
			const float fy = yr <= CIE_ETA ? (CIE_KAPPA * yr + 16) / 116.0f : std::cbrt(yr);
			const float fz = zr <= CIE_ETA ? (CIE_KAPPA * zr + 16) / 116.0f : std::cbrt(zr);

			const float L = 116 * fy - 16;
			const float A = 500 * (fx - fy);
			const float B = 200 * (fy - fz);

			r = L / 100.0f;			// [0, 100] => [0, 1]
			g = (A + 128) / 255.0f; // [-128, 127] => [0, 1]
			b = (B + 128) / 255.0f; // [-128, 127] => [0, 1]
		} break;
		case ColorFormat::LUV: {
			const float yr = y / REF_WHITE_Y;
			const float fy = yr <= CIE_ETA ? (CIE_KAPPA * yr + 16) / 116.0f : std::cbrt(yr);
			const float L  = 116 * fy - 16;

			const float uc = 4 * x / (x + 15 * y + 3 * z);
			const float vc = 9 * y / (x + 15 * y + 3 * z);

			const float ur = 4 * REF_WHITE_X / (REF_WHITE_X + 15 * REF_WHITE_Y + 3 * REF_WHITE_Z);
			const float vr = 9 * REF_WHITE_Y / (REF_WHITE_X + 15 * REF_WHITE_Y + 3 * REF_WHITE_Z);

			const float u = 13 * L * (uc - ur);
			const float v = 13 * L * (vc - vr);

			r = L / 100.0f;			// [0, 100] => [0, 1]
			g = (u + 100) / 200.0f; // [-100, 100] => [0, 1]
			b = (v + 100) / 200.0f; // [-100, 100] => [0, 1]
		} break;
		}
	}

	inline float tm_reinhard(float x) const { return x / (1 + x); }

	inline float tm_modified_reinhard(float x) const
	{
		constexpr float WhitePoint = 4.0f; // TODO: Make it a a parameter
		return x * (1 + x / (WhitePoint * WhitePoint)) / (1 + x);
	}

	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	inline float tm_aces(float x) const
	{
		float a = 2.51f;
		float b = 0.03f;
		float c = 2.43f;
		float d = 0.59f;
		float e = 0.14f;
		return x * std::fma(a, x, b) / std::fma(std::fma(c, x, d), x, e);
	}

	inline float mapTone(float x) const
	{
		switch (mToneMapping) {
		default:
		case ToneMappingMode::None:
			return x;
		case ToneMappingMode::Reinhard:
			return tm_reinhard(x);
		case ToneMappingMode::ModifiedReinhard:
			return tm_modified_reinhard(x);
		case ToneMappingMode::ACES:
			return tm_aces(x);
		}
	}

	inline float mapRange(float val) const
	{
		if (mMinRange < mMaxRange)
			return (clampRange(val) - mMinRange) / (mMaxRange - mMinRange);
		else
			return val;
	}

	float clamp01(float x) const { return std::max(0.0f, std::min(1.0f, x)); }
	float clampRange(float x) const { return std::max(mMinRange, std::min(mMaxRange, x)); }

	float applyExposureOffset(float x) const
	{
		return x * mExposureFactor + mOffset;
	}

	float applyGamma(float x) const
	{
		constexpr float AdobeRGBGamma = 2.19921875f;

		switch (mGammaType) {
		default:
		case GammaType::None:
			return x;
		case GammaType::SRGB:
			return x <= 0.0031308f ? x * 12.92f : (1.055f * std::pow(x, 1 / 2.4f) - 0.055f);
		case GammaType::ReverseSRGB:
			return x <= 0.04045f ? x / 12.92f : std::pow((x + 0.055f) / 1.055f, 2.4f);
		case GammaType::AdobeRGB:
			return std::pow(std::max(0.0f, x), 1 / AdobeRGBGamma);
		case GammaType::ReverseAdobeRGB:
			return std::pow(std::max(0.0f, x), AdobeRGBGamma);
		case GammaType::Custom:
			return std::pow(std::max(0.0f, x), mGamma);
		}
	}

	float mExposureFactor;
	float mOffset;
	float mMinRange;
	float mMaxRange;
	ColorFormat mTripletFormatSrc;
	ColorFormat mTripletFormatDst;
	ToneMappingMode mToneMapping;
	GammaType mGammaType;
	float mGamma;
};
} // namespace UI
} // namespace PR