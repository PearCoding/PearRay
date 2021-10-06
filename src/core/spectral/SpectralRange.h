#pragma once

#include "PR_Config.h"

namespace PR {

/// An interval class for spectral range with unbounded support
struct PR_LIB_CORE SpectralRange {
	float Start;
	float End;

	inline SpectralRange()
		: Start(-1)
		, End(-1)
	{
	}

	inline SpectralRange(float start, float end)
		: Start(start)
		, End(end)
	{
	}

	inline float span() const { return End - Start; }

	inline bool isStartUnbounded() const { return Start < 0; }
	inline void makeStartUnbounded() { Start = -1; }
	inline bool isEndUnbounded() const { return End < 0; }
	inline void makeEndUnbounded() { End = -1; }

	/// Will bound start and end by the given range if they are not bounded.
	/// This will not shrink the range except if unbounded!
	inline SpectralRange bounded(const SpectralRange& other) const
	{
		PR_ASSERT(!other.hasUnbounded(), "Expected other to be fully bounded!");
		return SpectralRange(
			isStartUnbounded() ? other.Start : Start,
			isEndUnbounded() ? other.End : End);
	}

	inline bool hasUnbounded() const { return isStartUnbounded() || isEndUnbounded(); }
	inline bool isInRange(float wvl) const { return wvl >= Start && (isEndUnbounded() || wvl <= End); }

	/// Map from [Start, End] to [0,1]
	/// Only works if bounded
	inline float mapToNormalized(float wvl) const
	{
		PR_ASSERT(!hasUnbounded(), "Range has to be bounded");
		return (wvl - Start) / span();
	}

	/// Map from [0,1] to [Start, End]
	/// Only works if bounded
	inline float mapFromNormalized(float wvl) const
	{
		PR_ASSERT(!hasUnbounded(), "Range has to be bounded");
		return wvl * span() + Start;
	}

	inline SpectralRange& operator+=(const SpectralRange& other)
	{
		Start = isStartUnbounded() ? other.Start : (other.isStartUnbounded() ? Start : std::min(Start, other.Start));
		End	  = std::max(End, other.End); // isEndUnbounded() ? other.End : (other.isEndUnbounded() ? End : std::max(End, other.End));

		return *this;
	}

	inline SpectralRange operator+(const SpectralRange& other) const
	{
		SpectralRange tmp = *this;
		tmp += other;
		return tmp;
	}
};

} // namespace PR
