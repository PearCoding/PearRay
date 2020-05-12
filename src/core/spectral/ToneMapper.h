#pragma once

#include "buffer/FrameBuffer.h"

namespace PR {
enum ToneColorMode {
	TCM_SRGB,
	TCM_XYZ,
	TCM_XYZ_NORM,
	TCM_LUMINANCE
};

enum ToneGammaMode {
	TGM_None,
	TGM_SRGB // 2.2
};

enum ToneMapperMode {
	TMM_None,
	TMM_Simple_Reinhard,
	TMM_Clamp,
	TMM_Abs,
	TMM_Positive,
	TMM_Negative,
	TMM_Spherical,
	TMM_Normalized
};

class SpectrumDescriptor;
class PR_LIB_CORE ToneMapper {
	PR_CLASS_NON_COPYABLE(ToneMapper);

public:
	ToneMapper();

	void map(const std::shared_ptr<SpectrumDescriptor>& desc,
			 const float* specIn,
			 float* rgbOut, size_t rgbElems, size_t pixelCount) const;
	void mapOnlyMapper(const float* rgbIn, float* rgbOut,
					   size_t rgbElems, size_t pixelCount) const;

	// Not thread safe!
	inline ToneColorMode colorMode() const { return mColorMode; }
	inline void setColorMode(ToneColorMode mode) { mColorMode = mode; }

	inline ToneGammaMode gammaMode() const { return mGammaMode; }
	inline void setGammaMode(ToneGammaMode mode) { mGammaMode = mode; }

	inline ToneMapperMode mapperMode() const { return mMapperMode; }
	inline void setMapperMode(ToneMapperMode mode) { mMapperMode = mode; }

private:
	ToneColorMode mColorMode;
	ToneGammaMode mGammaMode;
	ToneMapperMode mMapperMode;
};
} // namespace PR
