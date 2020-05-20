#pragma once

#include "shader/Socket.h"
#include "spectral/EquidistantSpectrum.h"
#include "spectral/ParametricBlob.h"

#include <sstream>

namespace PR {
template <typename T>
class PR_LIB_LOADER EquidistantSpectrumBaseMapSocket : public FloatSpectralMapSocket {
public:
	inline EquidistantSpectrumBaseMapSocket(const T& spec)
		: mSpectrum(spec)
	{
	}

	inline virtual SpectralBlob eval(const MapSocketCoord& ctx) const override
	{
		SpectralBlob blob;

		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k) {
			blob[k] = mSpectrum.lookup(ctx.WavelengthNM[k]);
		}
		return blob;
	}

	inline Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	inline virtual std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Spectrum (" << mSpectrum.sampleCount() << ", [" << mSpectrum.wavelengthStart() << ", " << mSpectrum.wavelengthEnd() << "])";
		return sstream.str();
	}

private:
	T mSpectrum;
};

using EquidistantSpectrumMapSocket	   = EquidistantSpectrumBaseMapSocket<EquidistantSpectrum>;
using EquidistantSpectrumViewMapSocket = EquidistantSpectrumBaseMapSocket<EquidistantSpectrumView>;

} // namespace PR
