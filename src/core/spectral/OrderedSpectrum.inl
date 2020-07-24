// IWYU pragma: private, include "spectral/OrderedSpectrum.h"

namespace PR {
inline OrderedSpectrumView::OrderedSpectrumView(const float* data, const float* wavelengths, size_t sample_count)
	: mData(data)
	, mWavelengths(wavelengths)
	, mSampleCount(sample_count)
{
}

inline float OrderedSpectrumView::lookup(float wavelength) const
{
	const int index = Interval::binary_search(mSampleCount, [&](int i) {
		return mWavelengths[i] <= wavelength;
	});
	const float t	= std::max(0.0f, std::min(1.0f, (wavelength - mWavelengths[index]) / (mWavelengths[index + 1] - mWavelengths[index])));

	return mData[index] * (1 - t) + mData[index + 1] * t;
}

} // namespace PR
