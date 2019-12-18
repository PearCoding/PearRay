#pragma once

#include <algorithm>
#include <cmath>

enum ColorFormat {
	CF_XYZ = 0,
	CF_SRGB
};
// Not the same as PR::ToneMapper!
class ToneMapper {
public:
	inline ToneMapper()
		: mMin(0.0f)
		, mMax(1.0f)
		, mIsAbsolute(false)
		, mTripletFormatFrom(CF_XYZ)
		, mTripletFormatTo(CF_XYZ)
		, mGamma(-1)
	{
	}
	~ToneMapper() = default;

	inline void setMax(float f) { mMax = f; }
	inline void setMin(float f) { mMin = f; }
	inline void enableAbsolute(bool b) { mIsAbsolute = b; }

	inline void setTripletFormat(ColorFormat formatFrom, ColorFormat formatTo)
	{
		mTripletFormatFrom = formatFrom;
		mTripletFormatTo   = formatTo;
	}

	inline void setGamma(float gamma) { mGamma = gamma; }

	inline void formatOnlyTriplet(float x, float y, float z, float& r, float& g, float& b) const
	{
		if (mTripletFormatFrom == mTripletFormatTo) {
			r = x;
			g = y;
			b = z;
		} else {
			float tx, ty, tz;
			formatTriplet(mTripletFormatFrom, x, y, z, tx, ty, tz);
			formatTriplet(mTripletFormatTo, tx, ty, tz, r, g, b);
		}
	}

	inline void mapTriplet(float x, float y, float z, float& r, float& g, float& b) const
	{
		if (mTripletFormatFrom == mTripletFormatTo) {
			r = x;
			g = y;
			b = z;
		} else {
			float tx, ty, tz;
			formatTriplet(mTripletFormatFrom, x, y, z, tx, ty, tz);
			formatTriplet(mTripletFormatTo, tx, ty, tz, r, g, b);
		}

		r = clamp(applyGamma(clamp(map(r))));
		g = clamp(applyGamma(clamp(map(g))));
		b = clamp(applyGamma(clamp(map(b))));
	}

private:
	void formatTriplet(ColorFormat format,
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
			float max = std::max(std::abs(mMin), std::abs(mMax));
			return std::min(std::abs(val), max) / max;
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
	ColorFormat mTripletFormatFrom;
	ColorFormat mTripletFormatTo;
	float mGamma;
};