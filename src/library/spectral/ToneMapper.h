#pragma once

#include "PR_Config.h"

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

class PR_LIB ToneMapper {
	PR_CLASS_NON_COPYABLE(ToneMapper);

public:
	/**
		 * @brief Constructs a ToneMapper
		 */
	ToneMapper(uint32 width, uint32 height);

	void map(const float* specIn, float* rgbOut, size_t specElems, size_t rgbElems) const;
	void mapOnlyMapper(const float* rgbIn, float* rgbOut, size_t rgbElems) const;

	// Not thread safe!
	inline ToneColorMode colorMode() const { return mColorMode; }
	inline void setColorMode(ToneColorMode mode) { mColorMode = mode; }

	inline ToneGammaMode gammaMode() const { return mGammaMode; }
	inline void setGammaMode(ToneGammaMode mode) { mGammaMode = mode; }

	inline ToneMapperMode mapperMode() const { return mMapperMode; }
	inline void setMapperMode(ToneMapperMode mode) { mMapperMode = mode; }

	inline uint32 width() const { return mWidth; }
	inline uint32 height() const { return mHeight; }

private:
	ToneColorMode mColorMode;
	ToneGammaMode mGammaMode;
	ToneMapperMode mMapperMode;

	uint32 mWidth;
	uint32 mHeight;
	size_t mSize;
};
}
