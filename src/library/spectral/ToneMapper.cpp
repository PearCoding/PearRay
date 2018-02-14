#include "ToneMapper.h"
#include "Spectrum.h"

#include "Logger.h"
#include "gpu/GPU.h"

#include "RGBConverter.h"
#include "XYZConverter.h"

constexpr float REINHARD_RATIO = 0.32f;
namespace PR {
ToneMapper::ToneMapper(uint32 width, uint32 height, GPU* gpu)
	: mColorMode(PR::TCM_SRGB)
	, mGammaMode(PR::TGM_SRGB)
	, mMapperMode(PR::TMM_Simple_Reinhard)
	, mGPU(gpu)
	, mWidth(width)
	, mHeight(height)
	, mSize(width * height)
{
#ifndef PR_NO_GPU
	if (!mGPU)
		return;

	mProgram = gpu->program("tonemapper");

	size_t fullSize = gpu->device().getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
	size_t maxSize  = (size_t)std::ceil((0.4f * fullSize) / (Spectrum::SAMPLING_COUNT * sizeof(float)));

	mRunSize = std::min(maxSize, size);
	try {
		mSpecInput = cl::Buffer(gpu->context(),
								CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
								mRunSize * Spectrum::SAMPLING_COUNT * sizeof(float));

		// Don't use runsize. Handle everything with only one call. It is an image after all.
		mInbetweenBuffer = cl::Buffer(gpu->context(),
									  CL_MEM_READ_WRITE | (!mByte ? CL_MEM_HOST_READ_ONLY : CL_MEM_HOST_NO_ACCESS),
									  size * 3 * sizeof(float));
	} catch (const cl::Error& error) {
		PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Error: %s (%s)", GPU::error(error.err()), error.what());
	}
#endif
}

void ToneMapper::map(const float* specIn, float* out, size_t specElems, size_t rgbElems) const
{
#ifndef PR_NO_GPU
	if (mGPU && mSize > 100) {
		try {
			cl::CommandQueue queue(mGPU->context(), mGPU->device(), 0);

			// Map 1 : Spec to Color RGB
			std::string specKernelName = "k_srgb";
			switch (mColorMode) {
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
			specKernel.setArg(3, (cl_ulong)rgbElems);

			for (size_t off = 0; off < mSize; off += mRunSize) {
				size_t current = std::min(mSize - off, mRunSize);
				specKernel.setArg(2, (cl_ulong)off);

				queue.enqueueWriteBuffer(mSpecInput, CL_TRUE,
										 0,
										 current * specElems * sizeof(float),
										 &in[off * specElems]);

				queue.enqueueNDRangeKernel(
					specKernel,
					cl::NullRange,
					cl::NDRange(current),
					cl::NullRange);
				queue.finish();
			}

			stage_mapper_gpu(queue, rgbElems);
			queue.finish();

			// Map 3: Gamma Correction
			if (mGammaMode != TGM_None) {
				cl::Kernel gammaKernel(mProgram, "k_gamma");
				gammaKernel.setArg(0, mInbetweenBuffer);
				gammaKernel.setArg(1, (cl_ulong)mSize * rgbElems);

				queue.enqueueNDRangeKernel(
					gammaKernel,
					cl::NullRange,
					cl::NDRange(mSize * rgbElems),
					cl::NullRange);
			}

			queue.finish();
			queue.enqueueReadBuffer(mInbetweenBuffer, CL_TRUE, 0,
									mSize * rgbElems * sizeof(float),
									out);
		} catch (const cl::Error& error) {
			PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Error: %s (%s)", GPU::error(error.err()), error.what());
		}
	} else {
#endif
		for (size_t i = 0; i < mSize; ++i) {
			float r, g, b;

			// Map 1: Spec to RGB
			switch (mColorMode) {
			case TCM_SRGB:
				RGBConverter::convert(specElems, &specIn[i * specElems], r, g, b);
				break;
			case TCM_XYZ:
				XYZConverter::convertXYZ(specElems, &specIn[i * specElems], r, g, b);
				break;
			case TCM_XYZ_NORM:
				XYZConverter::convert(specElems, &specIn[i * specElems], r, g);
				b = 1 - r - g;
				break;
			case TCM_LUMINANCE:
				RGBConverter::convert(specElems, &specIn[i * specElems], r, g, b);
				r = RGBConverter::luminance(r, g, b);
				g = r;
				b = r;
				break;
			}

			out[i * rgbElems]	 = r;
			out[i * rgbElems + 1] = g;
			out[i * rgbElems + 2] = b;
		}

		// Map 2: Tone Mapping
		stage_mapper_non_gpu(out, out, rgbElems);

		// Map 3: Gamma Correction
		if (mGammaMode != TGM_None) {
			for (size_t i = 0; i < mSize; ++i) {
				float r, g, b;
				r = out[i * rgbElems];
				g = out[i * rgbElems + 1];
				b = out[i * rgbElems + 2];

				RGBConverter::gamma(r, g, b);

				out[i * rgbElems]	 = r;
				out[i * rgbElems + 1] = g;
				out[i * rgbElems + 2] = b;
			}
		}
#ifndef PR_NO_GPU
	}
#endif
}

void ToneMapper::mapOnlyMapper(const float* rgbIn, float* rgbOut, size_t rgbElems) const
{
#ifndef PR_NO_GPU
	if (mGPU && mSize > 100) {
		try {
			cl::CommandQueue queue(mGPU->context(), mGPU->device(), 0);
			queue.enqueueWriteBuffer(mInbetweenBuffer, CL_TRUE,
									 0,
									 mSize * rgbElems * sizeof(float),
									 in);

			stage_mapper_gpu(queue, rgbElems);

			queue.finish();
			queue.enqueueReadBuffer(mInbetweenBuffer, CL_TRUE, 0,
									mSize * rgbElems * sizeof(float),
									out);
		} catch (const cl::Error& error) {
			PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Error: %s (%s)", GPU::error(error.err()), error.what());
		}
	} else {
#endif
		stage_mapper_non_gpu(rgbIn, rgbOut, rgbElems);
#ifndef PR_NO_GPU
	}
#endif
}

#ifndef PR_NO_GPU
void ToneMapper::stage_mapper_gpu(cl::CommandQueue& queue, size_t rgbElems) const
{
	switch (mMapperMode) {
	case TMM_None:
		break;
	case TMM_Simple_Reinhard: {
		cl::Kernel toneKernel(mProgram, "k_tone_reinhard_simple");
		toneKernel.setArg(0, mInbetweenBuffer);
		toneKernel.setArg(1, (cl_ulong)mSize);
		toneKernel.setArg(2, REINHARD_RATIO);
		toneKernel.setArg(3, (cl_ulong)rgbElems);

		queue.enqueueNDRangeKernel(
			toneKernel,
			cl::NullRange,
			cl::NDRange(mSize),
			cl::NullRange);
	} break;
	case TMM_Normalized: {
		float max = 0.0f;
		for (size_t i = 0; i < mSize; ++i) {
			float r, g, b;
			r = in[i * rgbElems];
			g = in[i * rgbElems + 1];
			b = in[i * rgbElems + 2];

			max = std::max(r * r + g * g + b * b, max);
		}
		max = std::sqrt(max);

		if (max <= PR_EPSILON)
			break;

		cl::Kernel toneKernel(mProgram, "k_tone_normalize");
		toneKernel.setArg(0, mInbetweenBuffer);
		toneKernel.setArg(1, (cl_ulong)mSize);
		toneKernel.setArg(2, 1.0f / max);
		toneKernel.setArg(3, (cl_ulong)rgbElems);

		queue.enqueueNDRangeKernel(
			toneKernel,
			cl::NullRange,
			cl::NDRange(mSize),
			cl::NullRange);
	} break;
	case TMM_Clamp:
	case TMM_Abs:
	case TMM_Positive:
	case TMM_Negative:
	case TMM_Spherical: {
		std::string kernelName = "k_tone_clamp";
		switch (mMapperMode) {
		case TMM_Abs:
			kernelName = "k_tone_abs";
			break;
		case TMM_Positive:
			kernelName = "k_tone_positive";
			break;
		case TMM_Negative:
			kernelName = "k_tone_negative";
			break;
		case TMM_Spherical:
			kernelName = "k_tone_spherical";
			break;
		default:
			break;
		}

		cl::Kernel toneKernel(mProgram, kernelName.c_str());
		toneKernel.setArg(0, mInbetweenBuffer);
		toneKernel.setArg(1, (cl_ulong)mSize);
		toneKernel.setArg(2, (cl_ulong)rgbElems);

		queue.enqueueNDRangeKernel(
			toneKernel,
			cl::NullRange,
			cl::NDRange(mSize),
			cl::NullRange);
	} break;
	}
}
#endif

void ToneMapper::stage_mapper_non_gpu(const float* rgbIn, float* rgbOut, size_t rgbElems) const
{
	float invMax = 0.0f;
	if (mMapperMode == TMM_Normalized) {
		for (size_t i = 0; i < mSize; ++i) {
			float r, g, b;
			r = rgbIn[i * rgbElems];
			g = rgbIn[i * rgbElems + 1];
			b = rgbIn[i * rgbElems + 2];

			invMax = std::max(r * r + g * g + b * b, invMax);
		}
		invMax = std::sqrt(invMax);

		if (invMax <= PR_EPSILON)
			return;

		invMax = 1.0f / invMax;
	}

	for (size_t i = 0; i < mSize; ++i) {
		float r, g, b;
		r = rgbIn[i * rgbElems];
		g = rgbIn[i * rgbElems + 1];
		b = rgbIn[i * rgbElems + 2];

		switch (mMapperMode) {
		case TMM_None:
			break;
		case TMM_Simple_Reinhard: {
			float Ld = 1 / (1 + RGBConverter::luminance(r, g, b) * REINHARD_RATIO);
			r *= Ld;
			g *= Ld;
			b *= Ld;
		} break;
		case TMM_Normalized:
			r *= invMax;
			g *= invMax;
			b *= invMax;
			break;
		case TMM_Clamp:
			r = std::max(std::abs(r), 0.0f);
			g = std::max(std::abs(g), 0.0f);
			b = std::max(std::abs(b), 0.0f);
			break;
		case TMM_Abs:
			r = std::abs(r);
			g = std::abs(g);
			b = std::abs(b);
			break;
		case TMM_Positive:
			r = std::max(r, 0.0f);
			g = std::max(g, 0.0f);
			b = std::max(b, 0.0f);
			break;
		case TMM_Negative:
			r = std::max(-r, 0.0f);
			g = std::max(-g, 0.0f);
			b = std::max(-b, 0.0f);
			break;
		case TMM_Spherical:
			r = 0.5f + 0.5f * std::atan2(b, r) * PR_1_PI;
			g = 0.5f - std::asin(-g) * PR_1_PI;
			b = 0;
			break;
		}

		rgbOut[i * rgbElems]	 = r;
		rgbOut[i * rgbElems + 1] = g;
		rgbOut[i * rgbElems + 2] = b;
	}
}
} // namespace PR
