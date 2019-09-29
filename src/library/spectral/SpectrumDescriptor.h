#pragma once

#include "Spectrum.h"
#include <vector>

namespace PR {
constexpr uint32 PR_SPECTRAL_WAVELENGTH_START   = 380; // nm
constexpr uint32 PR_SPECTRAL_WAVELENGTH_END		= 780; // nm
constexpr uint32 PR_SPECTRAL_WAVELENGTH_STEP	= 5;   // nm
constexpr uint32 PR_SPECTRAL_WAVELENGTH_SAMPLES = (PR_SPECTRAL_WAVELENGTH_END - PR_SPECTRAL_WAVELENGTH_START) / PR_SPECTRAL_WAVELENGTH_STEP + 1;

constexpr uint32 PR_SPECTRAL_TRIPLET_SAMPLES = 3;
// Based on D50 and sRGB
// FIXME: Have a better and configurable approach
constexpr float PR_SPECTRAL_TRIPLET_X_LAMBDA = 612.5f; // nm
constexpr float PR_SPECTRAL_TRIPLET_Y_LAMBDA = 546.8f; // nm
constexpr float PR_SPECTRAL_TRIPLET_Z_LAMBDA = 464.6f; // nm

class PR_LIB SpectrumDescriptor {
public:
	inline SpectrumDescriptor(uint32 samples, float lstart, float lend);
	inline explicit SpectrumDescriptor(const std::vector<float>& wavelengths);
	inline SpectrumDescriptor(const std::vector<float>& wavelengths, const std::vector<float>& lfactors);

	inline uint32 samples() const;

	inline const std::vector<float>& getWavelengths() const;
	inline const std::vector<float>& getLuminousFactors() const;

	inline float wavelength(uint32 index) const;
	inline void setWavelength(uint32 index, float lambda);

	inline float luminousFactor(uint32 index) const;
	inline void setLuminousFactor(uint32 index, float lambda);

	inline float integralDelta(uint32 index) const;

	inline bool isTriplet() const;
	inline bool isStandardSpectral() const;

	static std::shared_ptr<SpectrumDescriptor> createTriplet();
	static std::shared_ptr<SpectrumDescriptor> createStandardSpectral();

private:
	std::vector<float> mWavelengths;
	std::vector<float> mLuminousFactor;
};

} // namespace PR

#include "SpectrumDescriptor.inl"
