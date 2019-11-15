// IWYU pragma: private, include "spectral/SpectrumDescriptor.h"
namespace PR {
inline SpectrumDescriptor::SpectrumDescriptor(size_t samples, float lstart, float lend)
	: mWavelengths(samples)
	, mLuminousFactor(samples)
{
	PR_ASSERT(samples > 0, "Need sample count greater then 0");
	PR_ASSERT(lstart <= lend, "Need valid spectral start and end range");

	const float range = lend - lstart;
	const float step  = range / (samples - 1);
	for (size_t i = 0; i < samples; ++i) {
		mWavelengths[i] = lstart + i * step;
	}
}

inline SpectrumDescriptor::SpectrumDescriptor(const std::vector<float>& wavelengths)
	: mWavelengths(wavelengths)
	, mLuminousFactor(wavelengths.size())
{
}

inline SpectrumDescriptor::SpectrumDescriptor(const std::vector<float>& wavelengths, const std::vector<float>& lfactors)
	: mWavelengths(wavelengths)
	, mLuminousFactor(lfactors)
{
	PR_ASSERT(wavelengths.size() == lfactors.size(), "Amount of wavelengths and luminous factors have to be the same.");
}

inline size_t SpectrumDescriptor::samples() const
{
	return mWavelengths.size();
}

inline const std::vector<float>& SpectrumDescriptor::getWavelengths() const
{
	return mWavelengths;
}

inline const std::vector<float>& SpectrumDescriptor::getLuminousFactors() const
{
	return mLuminousFactor;
}

inline float SpectrumDescriptor::wavelength(size_t index) const
{
	return mWavelengths.at(index);
}

inline void SpectrumDescriptor::setWavelength(size_t index, float lambda)
{
	mWavelengths[index] = lambda;
}

inline float SpectrumDescriptor::luminousFactor(size_t index) const
{
	return mLuminousFactor.at(index);
}

inline void SpectrumDescriptor::setLuminousFactor(size_t index, float factor)
{
	mLuminousFactor[index] = factor;
}

inline float SpectrumDescriptor::integralDelta(size_t i) const
{
	PR_ASSERT(i < samples(), "Invalid integral delta call");

	if (i == 0) {								  // First Sample
		return wavelength(i + 1) - wavelength(i); // Approximation
	} else if (i == samples() - 1) {			  // Last Sample
		return wavelength(i) - wavelength(i - 1); // Approximation
	} else {
		return 0.5f * (wavelength(i + 1) - 2 * wavelength(i) + wavelength(i - 1));
	}
}

inline bool SpectrumDescriptor::isTriplet() const
{
	return samples() == PR_SPECTRAL_TRIPLET_SAMPLES;
}

inline bool SpectrumDescriptor::isStandardSpectral() const
{
	return samples() == PR_SPECTRAL_WAVELENGTH_SAMPLES;
}

} // namespace PR
