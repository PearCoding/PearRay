#pragma once

#include "Config.h"
#include "gpu/GPU.h"

namespace PR
{
	class GPU;
	class Spectrum;

	enum ToneColorMode
	{
		TCM_SRGB,
		TCM_XYZ,
		TCM_XYZ_NORM,
		TCM_LUMINANCE
	};

	enum ToneGammaMode
	{
		TGM_None,
		TGM_SRGB // 2.2
	};
	enum ToneMapperMode
	{
		TMM_None,
		TMM_Reinhard
	};
	class PR_LIB ToneMapper
	{
		PR_CLASS_NON_COPYABLE(ToneMapper);

	public:
		ToneMapper(GPU* gpu, size_t size, bool byte = false);

		// in -> size * SAMPLING_COUNT
		void exec(float* in, void* out);

		// Not thread safe!
		inline ToneColorMode colorMode() const
		{
			return mColorMode;
		}

		inline void setColorMode(ToneColorMode mode)
		{
			mColorMode = mode;
		}

		inline ToneGammaMode gammaMode() const
		{
			return mGammaMode;
		}

		inline void setGammaMode(ToneGammaMode mode)
		{
			mGammaMode = mode;
		}

		inline ToneMapperMode mapperMode() const
		{
			return mMapperMode;
		}

		inline void setMapperMode(ToneMapperMode mode)
		{
			mMapperMode = mode;
		}
	private:
		ToneColorMode mColorMode;
		ToneGammaMode mGammaMode;
		ToneMapperMode mMapperMode;

		GPU* mGPU;
		size_t mSize;
		bool mByte;

#ifndef PR_NO_GPU
		cl::Buffer mSpecInput;
		cl::Buffer mInbetweenBuffer;
		cl::Buffer mLocalBuffer;
		cl::Buffer mByteOutput;
		cl::Program mProgram;
		size_t mRunSize;
#endif
	};
}