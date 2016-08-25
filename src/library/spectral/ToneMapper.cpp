#include "ToneMapper.h"
#include "Spectrum.h"

#include "PearMath.h"
#include "gpu/GPU.h"
#include "Logger.h"

#include "RGBConverter.h"
#include "XYZConverter.h"

constexpr float REINHARD_RATIO = 0.32f;
namespace PR
{
	ToneMapper::ToneMapper(GPU* gpu, size_t size, bool byte) :
		mColorMode(PR::TCM_SRGB), mGammaMode(PR::TGM_SRGB), mMapperMode(PR::TMM_Simple_Reinhard),
		mGPU(gpu), mSize(size), mByte(byte)
	{
#ifndef PR_NO_GPU
		if (!mGPU)
			return;

		mProgram = gpu->program("tonemapper");

		size_t fullSize = gpu->device().getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
		size_t maxSize = (size_t)std::ceil((0.4f * fullSize) /
			(PR::Spectrum::SAMPLING_COUNT * sizeof(float)));

		mRunSize = PM::pm_MinT(maxSize, size);
		try
		{
			mSpecInput = cl::Buffer(gpu->context(),
				CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
				mRunSize * PR::Spectrum::SAMPLING_COUNT * sizeof(float)
			);

			// Don't use runsize. Handle everything with only one call. It is an image after all.
			mInbetweenBuffer = cl::Buffer(gpu->context(),
				CL_MEM_READ_WRITE | (!mByte ? CL_MEM_HOST_READ_ONLY : CL_MEM_HOST_NO_ACCESS),
				size * 3 * sizeof(float)
			);

			if (mByte)
			{
				mByteOutput = cl::Buffer(gpu->context(),
					CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
					size * 3 * sizeof(uint8)
				);
			}
		}
		catch (const cl::Error& error)
		{
			PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Error: %s (%s)", GPU::error(error.err()), error.what());
		}
#endif
	}

	void ToneMapper::exec(const float* in, void* out) const
	{
#ifndef PR_NO_GPU
		if (mGPU && mSize > 100)
		{
			try
			{
				cl::CommandQueue queue(mGPU->context(), mGPU->device(), 0);

				// Map 1 : Spec to Color RGB
				std::string specKernelName = "k_srgb";
				switch (mColorMode)
				{
				case TCM_SRGB:
					//specKernelName = "k_srgb";
					break;
				case TCM_XYZ:
					specKernelName = "k_xyz";
					break;
				case TCM_XYZ_NORM:
					specKernelName = "k_xyz_norm";
					break;
				case TCM_LUMINANCE:
					specKernelName = "k_lum";
					break;
				}

				cl::Kernel specKernel(mProgram, specKernelName.c_str());
				specKernel.setArg(0, mSpecInput);
				specKernel.setArg(1, mInbetweenBuffer);

				for (size_t off = 0; off < mSize; off += mRunSize)
				{
					size_t current = PM::pm_MinT(mSize - off, mRunSize);
					specKernel.setArg(2, (cl_ulong)off);

					queue.enqueueWriteBuffer(mSpecInput, CL_TRUE,
						0,
						current * PR::Spectrum::SAMPLING_COUNT * sizeof(float),
						&in[off * PR::Spectrum::SAMPLING_COUNT]);

					queue.enqueueNDRangeKernel(
						specKernel,
						cl::NullRange,
						cl::NDRange(current),
						cl::NullRange);
					queue.finish();
				}

				// Map 2: Tone Mapping
				switch (mMapperMode)
				{
				case TMM_None:
					break;
				case TMM_Simple_Reinhard:
				{
					cl::Kernel toneKernel(mProgram, "k_tone_reinhard_simple");
					toneKernel.setArg(0, mInbetweenBuffer);
					toneKernel.setArg(1, (cl_ulong)mSize);
					toneKernel.setArg(2, REINHARD_RATIO);

					queue.enqueueNDRangeKernel(
						toneKernel,
						cl::NullRange,
						cl::NDRange(mSize),
						cl::NullRange);
				}
				break;
				}

				// Map 3: Gamma Correction
				if (mGammaMode != TGM_None)
				{
					cl::Kernel gammaKernel(mProgram, "k_gamma");
					gammaKernel.setArg(0, mInbetweenBuffer);
					gammaKernel.setArg(1, (cl_ulong)mSize * 3);

					queue.enqueueNDRangeKernel(
						gammaKernel,
						cl::NullRange,
						cl::NDRange(mSize * 3),
						cl::NullRange);
				}

				// Map 4: Float to Byte
				if (mByte)
				{
					cl::Kernel byteKernel(mProgram, "k_to_byte");
					byteKernel.setArg(0, mInbetweenBuffer);
					byteKernel.setArg(1, mByteOutput);
					byteKernel.setArg(2, (cl_ulong)mSize*3);

					queue.enqueueNDRangeKernel(
						byteKernel,
						cl::NullRange,
						cl::NDRange(mSize*3),
						cl::NullRange);

					queue.finish();
					queue.enqueueReadBuffer(mByteOutput, CL_TRUE, 0,
						mSize * 3 * sizeof(uint8),
						out);
				}
				else
				{// No mapping needed, read float value directly
					queue.finish();
					queue.enqueueReadBuffer(mInbetweenBuffer, CL_TRUE, 0,
						mSize * 3 * sizeof(float),
						out);
				}
			}
			catch (const cl::Error& error)
			{
				PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Error: %s (%s)", GPU::error(error.err()), error.what());
			}
		}
		else
		{
#endif
			for (uint32 i = 0; i < mSize; ++i)
			{
				float r, g, b;
				// Map 1: Spec to RGB
				switch (mColorMode)
				{
				case TCM_SRGB:
					RGBConverter::convert(Spectrum(&in[i*Spectrum::SAMPLING_COUNT]),
						r, g, b);
					break;
				case TCM_XYZ:
					XYZConverter::convertXYZ(Spectrum(&in[i*Spectrum::SAMPLING_COUNT]),
						r, g, b);
					break;
				case TCM_XYZ_NORM:
					XYZConverter::convert(Spectrum(&in[i*Spectrum::SAMPLING_COUNT]),
						r, g, b);
					break;
				case TCM_LUMINANCE:
					RGBConverter::convert(Spectrum(&in[i*Spectrum::SAMPLING_COUNT]),
						r, g, b);
					r = RGBConverter::luminance(r, g, b);
					g = r; b = r;
					break;
				}

				// Map 2: Tone Mapping
				switch (mMapperMode)
				{
				case TMM_None:
					break;
				case TMM_Simple_Reinhard:
					{
						float Ld = 1 / (1 + RGBConverter::luminance(r, g, b) * REINHARD_RATIO);
						r *= Ld; g *= Ld; b *= Ld;
					}
					break;
				}

				// Map 3: Gamma Correction
				if (mGammaMode != TGM_None)
				{
					RGBConverter::gamma(r, g, b);
				}

				// Map 4: Float to Byte
				if (mByte)
				{
					reinterpret_cast<uint8*>(out)[i * 3] = (uint8)(PM::pm_ClampT(r, 0.0f, 1.0f) * 255);
					reinterpret_cast<uint8*>(out)[i * 3 + 1] = (uint8)(PM::pm_ClampT(g, 0.0f, 1.0f) * 255);
					reinterpret_cast<uint8*>(out)[i * 3 + 2] = (uint8)(PM::pm_ClampT(b, 0.0f, 1.0f) * 255);
				}
				else
				{
					reinterpret_cast<float*>(out)[i * 3] = r;
					reinterpret_cast<float*>(out)[i * 3 + 1] = g;
					reinterpret_cast<float*>(out)[i * 3 + 2] = b;
				}
			}
#ifndef PR_NO_GPU
		}
#endif
	}
}