#pragma once

#include "buffer/FrameBuffer.h"

namespace PR {
enum ToneColorMode {
	TCM_SRGB,
	TCM_XYZ,
	TCM_XYZ_NORM,
	TCM_LUMINANCE
};

class SpectrumDescriptor;
class PR_LIB_CORE ToneMapper {
	PR_CLASS_NON_COPYABLE(ToneMapper);

public:
	ToneMapper();

	void map(const std::shared_ptr<SpectrumDescriptor>& desc,
			 const float* specIn,
			 float* rgbOut, size_t rgbElems, size_t pixelCount) const;

	// Not thread safe!
	inline ToneColorMode colorMode() const { return mColorMode; }
	inline void setColorMode(ToneColorMode mode) { mColorMode = mode; }

private:
	ToneColorMode mColorMode;
};
} // namespace PR
