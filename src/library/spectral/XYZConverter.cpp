#include "XYZConverter.h"
#include "Spectrum.h"

#include "PearMath.h"
#include "gpu/GPU.h"
#include "Logger.h"

//#define PR_XYZ_LINEAR_INTERP

namespace PR
{
	// http://www.cs.utah.edu/~bes/papers/color/
	float NM_TO_X[Spectrum::SAMPLING_COUNT] = {
		0.00013, 0.00023, 0.00041, 0.00074, 0.00137, 0.00223, 0.00424, 0.00765, 0.01431, 0.02319,
		0.04351, 0.07763, 0.13438, 0.21477, 0.2839, 0.3285, 0.34828, 0.34806, 0.3362, 0.3187,
		0.2908, 0.2511, 0.19536, 0.1421, 0.09564, 0.05795, 0.03201, 0.0147, 0.0049, 0.0024,
		0.0093, 0.0291, 0.06327, 0.1096, 0.1655, 0.22575, 0.2904, 0.3597, 0.43345, 0.51205,
		0.5945, 0.6784, 0.7621, 0.8425, 0.9163, 0.9786, 1.0263, 1.0567, 1.0622, 1.0456,
		1.0026, 0.9384, 0.85445, 0.7514, 0.6424, 0.5419, 0.4479, 0.3608, 0.2835, 0.2187,
		0.1649, 0.1212, 0.0874, 0.0636, 0.04677, 0.0329, 0.0227, 0.01584, 0.01136, 0.00811,
		0.00579, 0.00411, 0.00289, 0.00205, 0.00144, 0.001, 0.00069, 0.00048, 0.00033, 0.00023,
		0.00017, 0.00012, 8e-05, 6e-05, 4.1e-05, 2.9e-05, 2e-05, 1.4e-05, 1e-05
	};

	float NM_TO_Y[Spectrum::SAMPLING_COUNT] = {
		0, 0, 1e-05, 2e-05, 4e-05, 6e-05, 0.00012, 0.00022, 0.0004, 0.00064,
		0.0012, 0.00218, 0.004, 0.0073, 0.0116, 0.01684, 0.023, 0.0298, 0.038, 0.048,
		0.06, 0.0739, 0.09098, 0.1126, 0.13902, 0.1693, 0.20802, 0.2586, 0.323, 0.4073,
		0.503, 0.6082, 0.71, 0.7932, 0.862, 0.91485, 0.954, 0.9803, 0.99495, 1,
		0.995, 0.9786, 0.952, 0.9154, 0.87, 0.8163, 0.757, 0.6949, 0.631, 0.5668,
		0.503, 0.4412, 0.381, 0.321, 0.265, 0.217, 0.175, 0.1382, 0.107, 0.0816,
		0.061, 0.04458, 0.032, 0.0232, 0.017, 0.01192, 0.00821, 0.00573, 0.0041, 0.00293,
		0.00209, 0.00105, 0.00105, 0.00074, 0.00052, 0.00036, 0.00025, 0.00017, 0.00012, 8e-05,
		6e-05, 4e-05, 3e-05, 2e-05, 1.4e-05, 1e-05, 7e-06, 5e-06, 3e-06
	};

	float NM_TO_Z[Spectrum::SAMPLING_COUNT] = {
		0.00061, 0.00108, 0.00195, 0.00349, 0.00645, 0.01055, 0.02005, 0.03621, 0.06785, 0.1102,
		0.2074, 0.3713, 0.6456, 1.03905, 1.3856, 1.62296, 1.74706, 1.7826, 1.77211, 1.7441,
		1.6692, 1.5281, 1.28764, 1.0419, 0.81295, 0.6162, 0.46518, 0.3533, 0.272, 0.2123,
		0.1582, 0.1117, 0.07825, 0.05725, 0.04216, 0.02984, 0.0203, 0.0134, 0.00875, 0.00575,
		0.0039, 0.00275, 0.0021, 0.0018, 0.00165, 0.0014, 0.0011, 0.001, 0.0008, 0.0006,
		0.00034, 0.00024, 0.00019, 0.0001, 5e-05, 3e-05, 2e-05, 1e-05, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	/* Source: https://upload.wikimedia.org/wikipedia/commons/8/8f/CIE_1931_XYZ_Color_Matching_Functions.svg */
	//float NM_TO_XYZ[Spectrum::SAMPLING_COUNT*3] = 
	//{/* X		Y		Z */
	//	0,		0,		0,		// 380 nm
	//	0.005f,	0,		0.05f,	// 390 nm [X Begin] [Z Begin]
	//	0.015f,	0,		0.08f,	// 400 nm 
	//	0.05f,	0,		0.18f,	// 410 nm 
	//	0.11f,	0,		0.55f,	// 420 nm
	//	0.29f,	0.005f,	1.22f,	// 430 nm [Y Begin]
	//	0.35f,	0.045f,	1.72f,	// 440 nm 
	//	0.33f,	0.055f,	1.725f,	// 450 nm 
	//	0.3f,	0.075f,	1.68f,	// 460 nm
	//	0.2f,	0.095f,	1.325f,	// 470 nm
	//	0.1f,	0.110f,	0.875f,	// 480 nm
	//	0.05f,	0.2f,	0.45f,	// 490 nm
	//	0.005f,	0.3f,	0.25f,	// 500 nm
	//	0.005f,	0.5f,	0.15f,	// 510 nm
	//	0.09f,	0.73f,	0.1f,	// 520 nm
	//	0.15f,	0.885f,	0.05f,	// 530 nm
	//	0.285f,	0.975f,	0.025f,	// 540 nm [Z End]
	//	0.43f,	1.0f,	0,		// 550 nm
	//	0.56f,	1.0f,	0,		// 560 nm
	//	0.74f,	0.96f,	0,		// 570 nm
	//	0.9f,	0.88f,	0,		// 580 nm
	//	1.025f,	0.725f,	0,		// 590 nm
	//	1.05f,	0.6f,	0,		// 600 nm
	//	0.98f,	0.49f,	0,		// 610 nm
	//	0.905f,	0.35f,	0,		// 620 nm
	//	0.65f,	0.25f,	0,		// 630 nm
	//	0.455f,	0.2f,	0,		// 640 nm
	//	0.245f,	0.11f,	0,		// 650 nm
	//	0.185f,	0.09f,	0,		// 660 nm
	//	0.1f,	0.05f,	0,		// 670 nm
	//	0.05f,	0.025f,	0,		// 680 nm [Y End]
	//	0.025f,	0,		0,		// 690 nm
	//	0.005f,	0,		0,		// 700 nm [X End]
	//	0,		0,		0,		// 710 nm
	//	0,		0,		0,		// 720 nm
	//	0,		0,		0,		// 730 nm
	//	0,		0,		0,		// 740 nm
	//	0,		0,		0,		// 750 nm
	//	0,		0,		0,		// 760 nm
	//	0,		0,		0,		// 770 nm
	//	0,		0,		0,		// 780 nm
	//};


	constexpr float ILL_SCALE = (Spectrum::WAVELENGTH_END - Spectrum::WAVELENGTH_START)
		/ (float)(Spectrum::SAMPLING_COUNT);// Nearly Spectrum::WAVELENGTH_STEP

	void XYZConverter::convertXYZ(const Spectrum& s, float &X, float &Y, float &Z)
	{
		X = 0;
		Y = 0;
		Z = 0;

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			float val1 = s.value(i);

#ifdef PR_XYZ_LINEAR_INTERP
			if (i < Spectrum::SAMPLING_COUNT - 1)
			{
				float val2 = s.value(i + 1);

				X += val1 * NM_TO_X[i] + val2 * NM_TO_X[i + 1];
				Y += val1 * NM_TO_Y[i] + val2 * NM_TO_Y[i + 1];
				Z += val1 * NM_TO_Z[i] + val2 * NM_TO_Z[i + 1];
			}
			else
			{
#endif
				X += val1 * NM_TO_X[i];
				Y += val1 * NM_TO_Y[i];
				Z += val1 * NM_TO_Z[i];
#ifdef PR_XYZ_LINEAR_INTERP
			}
#endif
		}

		X *= ILL_SCALE;
		Y *= ILL_SCALE;
		Z *= ILL_SCALE;

		if (!s.isEmissive())
		{
			X /= N;
			Y /= N;
			Z /= N;
		}

#ifdef PR_XYZ_LINEAR_INTERP
		X *= 0.5f;
		Y *= 0.5f;
		Z *= 0.5f;
#endif
	}

	void XYZConverter::convert(const Spectrum& s, float &x, float &y, float &z)
	{
		float X, Y, Z;
		convertXYZ(s, X, Y, Z);

		float m = X + Y + Z;
		if (m != 0)
		{
			x = X / m;
			y = Y / m;
			z = 1 - x - y;
		}
		else
		{
			x = 0; y = 0; z = 1;
		}
	}
	
	float XYZConverter::N = 0;
	void XYZConverter::init()
	{
		N = 0;
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			N += NM_TO_Y[i];
		}

		N *= ILL_SCALE;
	}

	XYZConverter::XYZConverter(GPU* gpu, size_t size, bool byte) :
		mGPU(gpu), mSize(size), mByte(byte)
	{
#ifndef PR_NO_GPU
		if (!mGPU)
			return;

		mProgram = gpu->program("rgbconvert");

		size_t fullSize = gpu->device().getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
		size_t maxSize = (0.4f * fullSize) /
			(PR::Spectrum::SAMPLING_COUNT * sizeof(float));

		mRunSize = PM::pm_MinT(maxSize, size);
		try
		{
			mInput = cl::Buffer(gpu->context(),
				CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
				mRunSize * PR::Spectrum::SAMPLING_COUNT * sizeof(float)
			);

			mOutput = cl::Buffer(gpu->context(),
				CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
				mRunSize * 3 * (mByte ? sizeof(uint8) : sizeof(float))
			);
		}
		catch (const cl::Error& error)
		{
			PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Error: %s (%s)", GPU::error(error.err()), error.what());
		}
#endif
	}

	void XYZConverter::convert(float* input, void* output, bool norm)
	{
#ifndef PR_NO_GPU
		if (mSize > 100)
		{
			try
			{
				cl::Kernel kernel(mProgram, norm ?
					(!mByte ? "m_xyz_norm" : "m_xyz_norm_byte") :
					(!mByte ? "m_xyz" : "m_xyz_byte"));
				kernel.setArg(0, mInput);
				kernel.setArg(1, mOutput);

				cl::CommandQueue queue(mGPU->context(), mGPU->device(), 0);

				for (size_t off = 0; off < mSize; off += mRunSize)
				{
					cl::Event event;
					size_t current = PM::pm_MinT(mSize - off, mRunSize);

					queue.enqueueWriteBuffer(mInput, CL_TRUE,
						0,
						current * PR::Spectrum::SAMPLING_COUNT * sizeof(float),
						&input[off * PR::Spectrum::SAMPLING_COUNT]);

					queue.enqueueNDRangeKernel(
						kernel,
						cl::NullRange,
						cl::NDRange(current),
						cl::NullRange,
						nullptr,
						&event);

					std::vector<cl::Event> events;
					events.push_back(event);

					queue.enqueueReadBuffer(mOutput, CL_TRUE,
						0,
						current * 3 * (mByte ? sizeof(uint8) : sizeof(float)),
						mByte ?
						(void*)&reinterpret_cast<uint8*>(output)[off * 3] :
						(void*)&reinterpret_cast<float*>(output)[off * 3],
						&events);
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
			if (norm)
			{
				if (mByte)
				{
					for (size_t i = 0; i < mSize; ++i)
					{
						float r, g, b;
						convert(Spectrum(&input[i*Spectrum::SAMPLING_COUNT]), r, g, b);

						reinterpret_cast<uint8*>(output)[i * 3] = PM::pm_ClampT(r, 0.0f, 1.0f) * 255;
						reinterpret_cast<uint8*>(output)[i * 3 + 1] = PM::pm_ClampT(g, 0.0f, 1.0f) * 255;
						reinterpret_cast<uint8*>(output)[i * 3 + 2] = PM::pm_ClampT(b, 0.0f, 1.0f) * 255;
					}
				}
				else
				{
					for (size_t i = 0; i < mSize; ++i)
					{
						convert(Spectrum(&input[i*Spectrum::SAMPLING_COUNT]),
							reinterpret_cast<float*>(output)[i * 3],
							reinterpret_cast<float*>(output)[i * 3 + 1],
							reinterpret_cast<float*>(output)[i * 3 + 2]);
					}
				}
			}
			else
			{
				if (mByte)
				{
					for (size_t i = 0; i < mSize; ++i)
					{
						float r, g, b;
						convertXYZ(Spectrum(&input[i*Spectrum::SAMPLING_COUNT]), r, g, b);

						reinterpret_cast<uint8*>(output)[i * 3] = PM::pm_ClampT(r, 0.0f, 1.0f) * 255;
						reinterpret_cast<uint8*>(output)[i * 3 + 1] = PM::pm_ClampT(g, 0.0f, 1.0f) * 255;
						reinterpret_cast<uint8*>(output)[i * 3 + 2] = PM::pm_ClampT(b, 0.0f, 1.0f) * 255;
					}
				}
				else
				{
					for (size_t i = 0; i < mSize; ++i)
					{
						convertXYZ(Spectrum(&input[i*Spectrum::SAMPLING_COUNT]),
							reinterpret_cast<float*>(output)[i * 3],
							reinterpret_cast<float*>(output)[i * 3 + 1],
							reinterpret_cast<float*>(output)[i * 3 + 2]);
					}
				}
			}
#ifndef PR_NO_GPU
		}
#endif
	}
}