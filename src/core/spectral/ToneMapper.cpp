#include "ToneMapper.h"
#include "Logger.h"
#include "RGBConverter.h"

namespace PR {
ToneMapper::ToneMapper()
	: mColorMode(TCM_SRGB)
{
}

void ToneMapper::map(const float* PR_RESTRICT xyzIn, float* PR_RESTRICT rgbOut, size_t outElems, size_t pixelCount) const
{
	constexpr size_t IElems = 3;
	PR_ASSERT(outElems >= 3, "Expected atleast an RGB buffer");
	PR_ASSERT(xyzIn != rgbOut, "Inplace tonemapping is not supported");

	switch (mColorMode) {
	case TCM_SRGB:
		RGBConverter::fromXYZ(xyzIn, rgbOut, outElems, pixelCount);
		return;
	case TCM_XYZ:
		if (outElems == 3) {
			memcpy(rgbOut, xyzIn, sizeof(float) * pixelCount * 3);
		} else {
			PR_OPT_LOOP
			for (size_t i = 0; i < pixelCount; ++i) {
				rgbOut[i * outElems + 0] = xyzIn[i * IElems + 0];
				rgbOut[i * outElems + 1] = xyzIn[i * IElems + 1];
				rgbOut[i * outElems + 2] = xyzIn[i * IElems + 2];
			}
		}
		return;
	case TCM_XYZ_NORM:
		PR_OPT_LOOP
		for (size_t i = 0; i < pixelCount; ++i) {
			const float X = xyzIn[i * IElems + 0];
			const float Y = xyzIn[i * IElems + 1];
			const float Z = xyzIn[i * IElems + 2];
			const float N = X + Y + Z;
			const float F = N != 0 ? 1.0f / N : 0;
			rgbOut[i * outElems + 0] *= F;
			rgbOut[i * outElems + 1] *= F;
			rgbOut[i * outElems + 2] *= F;
		}
		return;
	case TCM_LUMINANCE:
		PR_OPT_LOOP
		for (size_t i = 0; i < pixelCount; ++i) {
			const float Y			 = xyzIn[i * IElems + 1];
			rgbOut[i * outElems + 0] = Y;
			rgbOut[i * outElems + 1] = Y;
			rgbOut[i * outElems + 2] = Y;
		}
		return;
	}
}
} // namespace PR
