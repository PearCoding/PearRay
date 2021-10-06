#pragma once

#include "spectral/SpectralBlob.h"
#include "spectral/SpectralRange.h"

namespace PR {

inline SpectralBlob constructHeroWavelength(float hero, const SpectralRange& range)
{
	const float span  = range.span();
	const float delta = span / PR_SPECTRAL_BLOB_SIZE;
	const float start = hero - range.Start;

	SpectralBlob wvl;
	wvl(0) = hero; // Hero wavelength
	PR_OPT_LOOP
	for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		wvl(i) = range.Start + std::fmod(start + i * delta, span);

	return wvl;
}

inline SpectralBlob sampleStandardWavelength(float u, const SpectralRange& range, SpectralBlob& pdf)
{
	const float span  = range.span();
	const float delta = span / PR_SPECTRAL_BLOB_SIZE;
	const float start = u * span; // Wavelength inside the span

	SpectralBlob wvl;
	wvl(0) = start + range.Start; // Hero wavelength
	PR_OPT_LOOP
	for (size_t i = 1; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		wvl(i) = range.Start + std::fmod(start + i * delta, span);

	pdf = SpectralBlob::Ones();
	return wvl;
}

inline auto pdfStandardWavelength(const SpectralBlob& wvl, const SpectralRange& range)
{
	return wvl.unaryExpr([&](float f) {
		return range.isInRange(f) ? 1.0f : 0.0f;
	});
}
} // namespace PR