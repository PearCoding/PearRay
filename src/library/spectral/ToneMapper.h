#pragma once

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
		ToneMapper(uint32 width, uint32 height, GPU* gpu);

		void map(const Spectrum* specIn, float* rgbOut) const;
		void mapOnlyMapper(const float* rgbIn, float* rgbOut) const;

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

		GPU* mGPU;
		uint32 mWidth;
		uint32 mHeight;
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
