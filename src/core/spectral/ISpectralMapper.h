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
	Vector3f Position	   = Vector3f::Zero(); // Global position
	const PR::Light* Light = nullptr;

	SpectralSamplePurpose Purpose = SpectralSamplePurpose::Pixel;
};

struct PR_LIB_CORE SpectralSampleInput {
	Random& RND;
	Point2i Pixel;
	Vector3f Position	   = Vector3f::Zero(); // Global position
	const PR::Light* Light = nullptr;

	SpectralSamplePurpose Purpose = SpectralSamplePurpose::Pixel;

	inline explicit SpectralSampleInput(Random& rnd)
		: RND(rnd)
	{
	}
};

struct PR_LIB_CORE SpectralSampleOutput {
	float BlendWeight = 1; // Only valid for Pixel samples
	SpectralBlob WavelengthNM;
	SpectralBlob PDF;
};

class PR_LIB_CORE ISpectralMapper {
public:
	inline ISpectralMapper()
	{
	}

	virtual void sample(const SpectralSampleInput& in, SpectralSampleOutput& out) const = 0;

	virtual SpectralBlob pdf(const SpectralEvalInput& in, const SpectralBlob& wavelength) const
	{
		PR_UNUSED(in);
		PR_UNUSED(wavelength);
		return SpectralBlob::Ones();
	}
};

} // namespace PR