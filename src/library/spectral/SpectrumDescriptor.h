#pragma once

#include "Spectrum.h"
#include <vector>

namespace PR {
constexpr uint32 PR_SPECTRAL_WAVELENGTH_START	 = 380; // nm
constexpr uint32 PR_SPECTRAL_WAVELENGTH_END		 = 780; // nm
constexpr uint32 PR_SPECTRAL_WAVELENGTH_STEP	 = 5; // nm
constexpr uint32 PR_SPECTRAL_WAVELENGTH_SAMPLES	 = (PR_SPECTRAL_WAVELENGTH_END-PR_SPECTRAL_WAVELENGTH_START) / PR_SPECTRAL_WAVELENGTH_STEP + 1;
constexpr uint32 PR_SPECTRAL_TRIPLET_SAMPLES 	 = 3;


class PR_LIB SpectrumDescriptor {
public:
	inline SpectrumDescriptor(uint32 samples, float lstart, float lend);
	inline explicit SpectrumDescriptor(const std::vector<float>& wavelengths);
	inline SpectrumDescriptor(const std::vector<float>& wavelengths, const std::vector<float>& lfactors);
	
	inline uint32 samples() const;

	inline const std::vector<float>& get() const;

	inline float wavelength(uint32 index) const;
	inline void setWavelength(uint32 index, float lambda);

	inline float luminousFactor(uint32 index) const;
	inline void setLuminousFactor(uint32 index, float lambda);

	inline float integralDelta(uint32 index) const;
	
	inline Spectrum fromWhite() const;
	inline Spectrum fromBlack() const;
	Spectrum fromBlackbody(float temp) const; // Temp in Kelvin (K), Output W·sr^−1·m^−3

	inline bool isTriplet() const;
	inline bool isStandardSpectral() const;
private:
	std::vector<float> mWavelengths;
	std::vector<float> mLuminousFactor;
};

}

#include "SpectrumDescriptor.inl"
