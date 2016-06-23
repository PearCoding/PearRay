#pragma once

#include "Config.h"
#include "gpu/GPU.h"

namespace PR
{
	class GPU;
	class Spectrum;
	class PR_LIB XYZConverter
	{
		PR_CLASS_NON_COPYABLE(XYZConverter);

	public:
		/* E Illuminant */
		static void convert(const Spectrum& s, float &x, float &y, float &z);
		static void convertXYZ(const Spectrum& s, float &X, float &Y, float &Z);

		static void init();

		// Input size*SamplingCount to size*3
		XYZConverter(GPU* gpu, size_t size, bool byte = false);
		void convert(float* input, void* output, bool norm = false);

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

		static float N;// Illuminent Factor
	};
}