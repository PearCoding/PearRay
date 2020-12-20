#pragma once

#include <algorithm>
#include <cmath>

#include "PR_Config.h"

namespace PR {
namespace UI {
enum ColorFormat {
	CF_XYZ = 0,
	CF_SRGB
};

/* Pipeline:
 (1) Src to XYZ
 (2) Exposure + Offset
 (3) Range
 (4) XYZ to Dst
 (5) Gamma
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
		, mGamma(-1)
	{
	}
	~ImagePipeline() = default;

	inline void setExposure(float exposure) { mExposureFactor = std::pow(2, exposure); }
	inline void setOffset(float offset) { mOffset = offset; }
	inline void setMaxRange(float f) { mMaxRange = f; }
	inline void setMinRange(float f) { mMinRange = f; }

	inline void setTripletFormat(ColorFormat formatFrom, ColorFormat formatTo)
	{
		mTripletFormatSrc = formatFrom;
		mTripletFormatDst = formatTo;
	}

	inline void setGamma(float gamma) { mGamma = gamma; }

	inline void mapTriplet(float x, float y, float z, float& r, float& g, float& b) const
	{
		// (1)
		formatTripletToXYZ(mTripletFormatSrc, x, y, z, r, g, b);

		// (2)+(3)
		x = mapRange(applyExposureOffset(r));
		y = mapRange(applyExposureOffset(g));
		z = mapRange(applyExposureOffset(b));

		// (4)
		formatTripletFromXYZ(mTripletFormatDst, x, y, z, r, g, b);

		// (5)
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
	float mGamma;
};
} // namespace UI
} // namespace PR