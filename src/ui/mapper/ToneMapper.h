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

// Not the same as PR::ToneMapper!
class PR_LIB_UI ToneMapper {
public:
	inline ToneMapper()
		: mMin(0.0f)
		, mMax(1.0f)
		, mIsAbsolute(false)
		, mTripletFormatSrc(CF_XYZ)
		, mTripletFormatDst(CF_XYZ)
		, mGamma(-1)
	{
	}
	~ToneMapper() = default;

	inline void setMax(float f) { mMax = f; }
	inline void setMin(float f) { mMin = f; }
	inline void enableAbsolute(bool b) { mIsAbsolute = b; }

	inline void setTripletFormat(ColorFormat formatFrom, ColorFormat formatTo)
	{
		mTripletFormatSrc = formatFrom;
		mTripletFormatDst = formatTo;
	}

	inline void setGamma(float gamma) { mGamma = gamma; }

	inline void formatOnlyTriplet(float x, float y, float z, float& r, float& g, float& b) const
	{
		if (mTripletFormatSrc == mTripletFormatDst) {
			r = x;
			g = y;
			b = z;
		} else {
			float tx, ty, tz;
			formatTripletToXYZ(mTripletFormatSrc, x, y, z, tx, ty, tz);
			formatTripletFromXYZ(mTripletFormatDst, tx, ty, tz, r, g, b);
		}
	}

	inline void mapTriplet(float x, float y, float z, float& r, float& g, float& b) const
	{
		formatOnlyTriplet(x, y, z, r, g, b);
		r = clamp(applyGamma(clamp(map(r))));
		g = clamp(applyGamma(clamp(map(g))));
		b = clamp(applyGamma(clamp(map(b))));
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
			x = 4.123908e-01f * r + 3.575843e-01f * g + 1.804808e-01f * b;
			y = 2.126390e-01f * r + 7.151687e-01f * g + 7.219232e-02f * b;
			z = 1.933082e-02f * r + 1.191948e-01f * g + 9.505322e-01f * b;
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
			r = 3.240970e+00f * x - 1.537383e+00f * y - 4.986108e-01f * z;
			g = -9.692436e-01f * x + 1.875968e+00f * y + 4.155506e-02f * z;
			b = 5.563008e-02f * x - 2.039770e-01f * y + 1.056972e+00f * z;
			break;
		}
	}

	inline float map(float val) const
	{
		if (mIsAbsolute) {
			float min = std::max(0.0f, mMin);
			float max = std::max(std::abs(mMin), std::abs(mMax));
			return std::min(std::max(min, std::abs(val)) - min, max) / (max - min);
		} else {
			return std::abs((std::min(mMax, std::max(mMin, val)) - mMin) / (mMax - mMin));
		}
	}

	float clamp(float x) const { return std::max(0.0f, std::min(1.0f, x)); }

	float applyGamma(float x) const
	{
		return mGamma <= 0.0f ? x : std::pow(x, mGamma);
	}

	float mMin;
	float mMax;
	bool mIsAbsolute;
	ColorFormat mTripletFormatSrc;
	ColorFormat mTripletFormatDst;
	float mGamma;
};
} // namespace UI
} // namespace PR