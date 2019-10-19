#pragma once

#include <algorithm>

// Not the same as PR::ToneMapper!
class ToneMapper {
public:
	inline ToneMapper()
		: mMin(0.0f)
		, mMax(1.0f)
		, mIsAbsolute(false)
	{
	}
	~ToneMapper() = default;

	inline float map(float val) const
	{
		if (mIsAbsolute) {
			float max = std::max(std::abs(mMin), std::abs(mMax));
			return std::min(std::abs(val), max) / max;
		} else {
			return std::abs((std::min(mMax, std::max(mMin, val)) - mMin) / (mMax - mMin));
		}
	}

	inline void setMax(float f) { mMax = f; }
	inline void setMin(float f) { mMin = f; }
	inline void enableAbsolute(bool b) { mIsAbsolute = b; }

private:
	float mMin;
	float mMax;
	bool mIsAbsolute;
};