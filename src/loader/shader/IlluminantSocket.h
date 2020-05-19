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

#define _ILLUMINANT(Prefix)                                                       \
	class PR_LIB_LOADER Prefix##Illuminant : public IlluminantSpectralMapSocket { \
	public:                                                                       \
		explicit Prefix##Illuminant(float power = 1.0f);                          \
		SpectralBlob eval(const MapSocketCoord& ctx) const override;              \
		std::string dumpInformation() const override;                             \
                                                                                  \
	private:                                                                      \
		float mPower;                                                             \
	};

_ILLUMINANT(D65)
_ILLUMINANT(D50)
_ILLUMINANT(D55)
_ILLUMINANT(D75)
_ILLUMINANT(A)
_ILLUMINANT(C)
_ILLUMINANT(F1)
_ILLUMINANT(F2)
_ILLUMINANT(F3)
_ILLUMINANT(F4)
_ILLUMINANT(F5)
_ILLUMINANT(F6)
_ILLUMINANT(F7)
_ILLUMINANT(F8)
_ILLUMINANT(F9)
_ILLUMINANT(F10)
_ILLUMINANT(F11)
_ILLUMINANT(F12)

#undef _ILLUMINANT
} // namespace PR
