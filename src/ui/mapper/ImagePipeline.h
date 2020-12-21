#pragma once

#include <algorithm>
#include <cmath>

#include "PR_Config.h"

namespace PR {
namespace UI {
enum ColorFormat {
	CF_XYZ = 0,
	CF_SRGB,
	CF_LAB,
	CF_LUV
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

enum ToneMapping {
	TM_None = 0,
	TM_Reinhard,
	TM_ModifiedReinhard,
	TM_ACES
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
		, mTripletFormatSrc(CF_XYZ)
		, mTripletFormatDst(CF_XYZ)
		, mToneMapping(TM_None)
		, mGamma(-1)
	{
	}
	~ImagePipeline() = default;

	inline void setExposure(float exposure) { mExposureFactor = std::pow(2, exposure); }
	inline void setOffset(float offset) { mOffset = offset; }
	inline void setMaxRange(float f) { mMaxRange = f; }
	inline void setMinRange(float f) { mMinRange = f; }
	inline void setToneMapping(ToneMapping tm) { mToneMapping = tm; }

	inline void setTripletFormat(ColorFormat formatFrom, ColorFormat formatTo)
	{
		mTripletFormatSrc = formatFrom;
		mTripletFormatDst = formatTo;
	}

	inline void setGamma(float gamma) { mGamma = gamma; }

	inline void mapTriplet(float x, float y, float z, float& r, float& g, float& b) const
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
		r = clamp01(applyGamma(r));
		g = clamp01(applyGamma(g));
		b = clamp01(applyGamma(b));
	}

private:
	void formatTripletToXYZ(ColorFormat format,
							float r, float g, float b,
							float& x, float& y, float& z) const
	{
		switch (format) {
		default:
		case CF_XYZ:
			x = r;
			y = g;
			z = b;
			break;
		case CF_SRGB:
			x = +4.123908e-01f * r + 3.575843e-01f * g + 1.804808e-01f * b;
			y = +2.126390e-01f * r + 7.151687e-01f * g + 7.219232e-02f * b;
			z = +1.933082e-02f * r + 1.191948e-01f * g + 9.505322e-01f * b;
			break;
		case CF_LAB: {
		} break;
		case CF_LUV: {
		} break;
		}
	}

	void formatTripletFromXYZ(ColorFormat format,
							  float x, float y, float z,
							  float& r, float& g, float& b) const
	{
		switch (format) {
		default:
		case CF_XYZ:
			r = x;
			g = y;
			b = z;
			break;
		case CF_SRGB:
			r = +3.240970e+00f * x - 1.537383e+00f * y - 4.986108e-01f * z;
			g = -9.692436e-01f * x + 1.875968e+00f * y + 4.155506e-02f * z;
			b = +5.563008e-02f * x - 2.039770e-01f * y + 1.056972e+00f * z;
			break;
		case CF_LAB: {
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
		case CF_LUV: {
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
		case TM_None:
			return x;
		case TM_Reinhard:
			return tm_reinhard(x);
		case TM_ModifiedReinhard:
			return tm_modified_reinhard(x);
		case TM_ACES:
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
		return mGamma <= 0.0f ? x : std::pow(std::max(0.0f, x), mGamma);
	}

	float mExposureFactor;
	float mOffset;
	float mMinRange;
	float mMaxRange;
	ColorFormat mTripletFormatSrc;
	ColorFormat mTripletFormatDst;
	ToneMapping mToneMapping;
	float mGamma;
};
} // namespace UI
} // namespace PR