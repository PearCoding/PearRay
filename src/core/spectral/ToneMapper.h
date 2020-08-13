#pragma once

#include "buffer/FrameBuffer.h"

namespace PR {
enum ToneColorMode {
	TCM_SRGB,
	TCM_XYZ,
	TCM_XYZ_NORM,
	TCM_LUMINANCE
};

/// Maps CIE XYZ triplets to another arbitary color space
class PR_LIB_CORE ToneMapper {
	PR_CLASS_NON_COPYABLE(ToneMapper);

public:
	ToneMapper();

	/// Maps xyz buffer to rgb buffer. XYZ buffer is of size pixelCount*3, rgb is pixelCount*outElems
	void map(const float* PR_RESTRICT xyzIn, float* PR_RESTRICT rgbOut, size_t outElems, size_t pixelCount) const;

	// Not thread safe!
	inline ToneColorMode colorMode() const { return mColorMode; }
	inline void setColorMode(ToneColorMode mode) { mColorMode = mode; }

	inline float scale() const { return mScale; }
	inline void setScale(float s) { mScale = s; }

private:
	ToneColorMode mColorMode;
	float mScale;
};
} // namespace PR
