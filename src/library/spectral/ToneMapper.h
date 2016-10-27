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
		TMM_Simple_Reinhard,
		TMM_Clamp,
		TMM_Abs,
		TMM_Positive,
		TMM_Negative,
		TMM_Spherical,
		TMM_Normalized
	};

	class PR_LIB ToneMapper
	{
		PR_CLASS_NON_COPYABLE(ToneMapper);

	public:
		/**
		 * @brief Constructs a ToneMapper
		 * @param gpu GPU instance to be used, nullptr if no gpu support.
		 * @param size Size of buffer (without component count -> only width*height but no rgb [3])
		 */
		ToneMapper(GPU* gpu, size_t size);

		// in -> size * SAMPLING_COUNT
		void exec(const float* specIn, float* rgbOut) const;

		void execMapper(const float* rgbIn, float* rgbOut) const;

		// Not thread safe!
		inline ToneColorMode colorMode() const { return mColorMode; }
		inline void setColorMode(ToneColorMode mode) { mColorMode = mode; }

		inline ToneGammaMode gammaMode() const { return mGammaMode; }
		inline void setGammaMode(ToneGammaMode mode) { mGammaMode = mode; }

		inline ToneMapperMode mapperMode() const { return mMapperMode; }
		inline void setMapperMode(ToneMapperMode mode) { mMapperMode = mode; }

		ToneColorMode mColorMode;
		ToneGammaMode mGammaMode;
		ToneMapperMode mMapperMode;

		GPU* mGPU;
		size_t mSize;

#ifndef PR_NO_GPU
		cl::Buffer mSpecInput;
		cl::Buffer mInbetweenBuffer;
		cl::Buffer mLocalBuffer;
		cl::Program mProgram;
		size_t mRunSize;

		void stage_mapper_gpu(cl::CommandQueue& queue) const;
#endif
		void stage_mapper_non_gpu(const float* rgbIn, float* rgbOut) const;
	};
}