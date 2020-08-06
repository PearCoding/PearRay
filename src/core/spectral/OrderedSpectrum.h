#pragma once

#include "container/Interval.h"

namespace PR {
// The wavelength has to be ordered
class PR_LIB_CORE OrderedSpectrumView {
public:
	inline OrderedSpectrumView(const float* PR_RESTRICT data, const float* PR_RESTRICT wavelengths, size_t sample_count);
	~OrderedSpectrumView() = default;

	inline const float* data() const { return mData; }
	inline const float* wavelengths() const { return mWavelengths; }

	inline float lookup(float wavelength) const;

	inline size_t sampleCount() const { return mSampleCount; }
	inline float wavelengtStart() const { return mWavelengths[0]; }
	inline float wavelengtEnd() const { return mWavelengths[mSampleCount - 1]; }

private:
	const float* PR_RESTRICT mData;
	const float* PR_RESTRICT mWavelengths;
	const size_t mSampleCount;
};
// TODO: Owning variant
} // namespace PR

#include "OrderedSpectrum.inl"