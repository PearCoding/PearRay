#pragma once

#include "SpectralBlob.h"

namespace PR {

struct PR_LIB_CORE SpectralMapSample {
	SpectralBlob BlendWeight = SpectralBlob::Ones();
	SpectralBlob WavelengthNM;
	SpectralBlob PDF;
};

class RenderTile;
class StreamPipeline;
struct OutputSpectralEntry;
class PR_LIB_CORE ISpectralMapper {
public:
	inline ISpectralMapper(float start, float end)
		: mStart(start)
		, mEnd(end)
	{
	}

	virtual SpectralMapSample sample(const Point2i& pixel, float u) const = 0;

	virtual SpectralBlob pdf(const Point2i& pixel, const SpectralBlob& wavelength) const
	{
		PR_UNUSED(pixel);
		PR_UNUSED(wavelength);
		return SpectralBlob::Ones();
	}

	inline float wavelengthStart() const { return mStart; }
	inline float wavelengthEnd() const { return mEnd; }
	inline bool isMonochrome() const { return mStart == mEnd; }

private:
	const float mStart;
	const float mEnd;
};

} // namespace PR