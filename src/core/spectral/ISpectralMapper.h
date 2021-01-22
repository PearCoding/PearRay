#pragma once

#include "SpectralBlob.h"

namespace PR {

class Light;
enum class SpectralSamplePurpose {
	Pixel,		// Light and Position are not valid
	Connection, // Position has to be set
	Light		// Position and Light is set
};

struct PR_LIB_CORE SpectralEvalInput {
	Point2i Pixel;
	Vector3f Position = Vector3f::Zero(); // Global position
	PR::Light* Light  = nullptr;

	SpectralSamplePurpose Purpose = SpectralSamplePurpose::Pixel;
};

struct PR_LIB_CORE SpectralSampleInput {
	Random& RND;
	Point2i Pixel;
	Vector3f Position = Vector3f::Zero(); // Global position
	PR::Light* Light  = nullptr;

	SpectralSamplePurpose Purpose = SpectralSamplePurpose::Pixel;

	inline explicit SpectralSampleInput(Random& rnd)
		: RND(rnd)
	{
	}
};

struct PR_LIB_CORE SpectralSampleOutput {
	SpectralBlob BlendWeight = SpectralBlob::Ones(); // Only valid for Pixel samples
	SpectralBlob WavelengthNM;
	SpectralBlob PDF;
};

class PR_LIB_CORE ISpectralMapper {
public:
	inline ISpectralMapper(float start, float end)
		: mStart(start)
		, mEnd(end)
	{
	}

	virtual void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const = 0;

	virtual SpectralBlob pdf(const SpectralEvalInput& in, const SpectralBlob& wavelength) const
	{
		PR_UNUSED(in);
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