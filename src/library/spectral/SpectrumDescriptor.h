#pragma once

#include "Spectrum.h"
#include <vector>

namespace PR {
constexpr size_t PR_SPECTRAL_WAVELENGTH_START   = 380; // nm
constexpr size_t PR_SPECTRAL_WAVELENGTH_END		= 780; // nm
constexpr size_t PR_SPECTRAL_WAVELENGTH_STEP	= 5;   // nm
constexpr size_t PR_SPECTRAL_WAVELENGTH_SAMPLES = (PR_SPECTRAL_WAVELENGTH_END - PR_SPECTRAL_WAVELENGTH_START) / PR_SPECTRAL_WAVELENGTH_STEP + 1;

constexpr size_t PR_SPECTRAL_TRIPLET_SAMPLES = 3;
// Based on D50 and CIE XYZ
// FIXME: Have a better and configurable approach
constexpr float PR_SPECTRAL_TRIPLET_X_LAMBDA = -500.2f; // nm
constexpr float PR_SPECTRAL_TRIPLET_Y_LAMBDA = 522.1f; // nm
constexpr float PR_SPECTRAL_TRIPLET_Z_LAMBDA = 477.1f; // nm

class PR_LIB SpectrumDescriptor {
public:
	inline SpectrumDescriptor(size_t samples, float lstart, float lend);
	inline explicit SpectrumDescriptor(const std::vector<float>& wavelengths);
	inline SpectrumDescriptor(const std::vector<float>& wavelengths, const std::vector<float>& lfactors);

	inline size_t samples() const;

	inline const std::vector<float>& getWavelengths() const;
	inline const std::vector<float>& getLuminousFactors() const;

	inline float wavelength(size_t index) const;
	inline void setWavelength(size_t index, float lambda);

	inline float luminousFactor(size_t index) const;
	inline void setLuminousFactor(size_t index, float lambda);

	inline float integralDelta(size_t index) const;

	inline bool isTriplet() const;
	inline bool isStandardSpectral() const;

	static std::shared_ptr<SpectrumDescriptor> createTriplet();
	static std::shared_ptr<SpectrumDescriptor> createStandardSpectral();
	static std::shared_ptr<SpectrumDescriptor> createDefault();

private:
	std::vector<float> mWavelengths;
	std::vector<float> mLuminousFactor;
};

} // namespace PR

#include "SpectrumDescriptor.inl"
