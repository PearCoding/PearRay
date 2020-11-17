#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {
namespace VCM {
// TODO
constexpr float WAVELENGTH_START = 360;
constexpr float WAVELENGTH_END	 = 760;
constexpr float WAVEBAND		 = 5;
inline SpectralBlob sampleWavelength(Random& rnd)
{
	constexpr float span  = WAVELENGTH_END - WAVELENGTH_START;
	constexpr float delta = span / PR_SPECTRAL_BLOB_SIZE;

	SpectralBlob wvls;
	const float start = rnd.getFloat() * span;	  // Wavelength inside the span
	wvls(0)			  = start + WAVELENGTH_START; // Hero wavelength
	PR_OPT_LOOP
	for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		wvls(i) = WAVELENGTH_START + std::fmod(start + i * delta, span);
	return wvls;
}

// TODO
inline bool wavelengthCheck(float a, float b)
{
	return std::abs(a - b) <= (WAVEBAND / 2);
}

inline float wavelengthFilter(float a, float b)
{
	constexpr float PDF = 1 - 0.5f * WAVEBAND / (WAVELENGTH_END - WAVELENGTH_START); // Area of box[-w/2,w/2] x box[0,1] convolution
	const bool check	= wavelengthCheck(a, b);
	return check ? 1.0f / PDF : 0.0f;
}

using SpectralPermutation = std::array<size_t, PR_SPECTRAL_BLOB_SIZE>;
inline bool checkWavelengthSupport(const SpectralBlob& wvlA, bool monoA, const SpectralBlob& wvlB, bool monoB, SpectralPermutation& permutation)
{
	const size_t sA = monoA ? 1 : PR_SPECTRAL_BLOB_SIZE;
	const size_t sB = monoB ? 1 : PR_SPECTRAL_BLOB_SIZE;

	bool anyGood = false;
	std::array<bool, PR_SPECTRAL_BLOB_SIZE> maskA;
	std::array<bool, PR_SPECTRAL_BLOB_SIZE> maskB;
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
		maskA[i] = false;
		maskB[i] = false;
	}

	// Select first fitting wavelength
	// TODO: Why not best fitting?
	for (size_t i = 0; i < sA; ++i) {
		for (size_t j = 0; j < sB; ++j) {
			if (maskB[j])
				continue;

			if (wavelengthCheck(wvlA[i], wvlB[j])) {
				anyGood		   = true;
				maskA[i]	   = true;
				maskB[j]	   = true;
				permutation[i] = j;
				break;
			}
		}
	}

	// Make sure the permutation vector is still filled reasonable
	if (anyGood) {
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			if (maskA[i])
				continue;
			for (size_t j = 0; j < PR_SPECTRAL_BLOB_SIZE; ++j) {
				if (maskB[j])
					continue;
				maskA[i]	   = true;
				maskB[j]	   = true;
				permutation[i] = j;
				break;
			}
		}
	}

	return anyGood;
}

} // namespace VCM
} // namespace PR