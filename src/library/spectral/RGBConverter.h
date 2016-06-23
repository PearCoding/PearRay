#pragma once

#include "Spectrum.h"

#include "gpu/GPU.h"

namespace PR
{
	class GPU;
	class PR_LIB RGBConverter
	{
		PR_CLASS_NON_COPYABLE(RGBConverter);

	public:
		/* D65 sRGB (linear) */
		static void convert(const Spectrum& s, float &x, float &y, float &z);

		/* D65 sRGB (sRGB gamma) */
		static void convertGAMMA(const Spectrum& s, float &x, float &y, float &z);

		static Spectrum toSpec(float x, float y, float z);
		static void init();

		static Spectrum White;
		static Spectrum Cyan;
		static Spectrum Magenta;
		static Spectrum Yellow;
		static Spectrum Red;
		static Spectrum Green;
		static Spectrum Blue;

		// Input size*SamplingCount to size*3
		RGBConverter(GPU* gpu, size_t size, bool byte = false);
		void convert(float* input, void* output, bool linear = false);

	private:
		GPU* mGPU;
		size_t mSize;
		bool mByte;
#ifndef PR_NO_GPU
		cl::Buffer mInput;
		cl::Buffer mOutput;
		cl::Program mProgram;
		size_t mRunSize;
#endif
	};
}