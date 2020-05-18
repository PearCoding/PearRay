#pragma once

#include "shader/Socket.h"
#include "spectral/ParametricBlob.h"

namespace PR {

class PR_LIB_LOADER IlluminantSpectralMapSocket : public FloatSpectralMapSocket {
public:
	IlluminantSpectralMapSocket(const float* data, size_t sample_count, float wavelength_start, float wavelength_end);
	virtual ~IlluminantSpectralMapSocket() = default;

	virtual SpectralBlob eval(const MapSocketCoord& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	virtual std::string dumpInformation() const override;

private:
	const float* mData;
	const size_t mSampleCount;
	const float mWavelengthStart;
	const float mWavelengthEnd;
	const float mWavelengthDelta;
};

class PR_LIB_LOADER D65Illuminant : public IlluminantSpectralMapSocket {
public:
	explicit D65Illuminant(float power=1.0f);

	SpectralBlob eval(const MapSocketCoord& ctx) const override;
	std::string dumpInformation() const override;

	private:
	float mPower;
};

} // namespace PR
