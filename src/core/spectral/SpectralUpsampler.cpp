#include "SpectralUpsampler.h"
#include "serialization/Serializer.h"

#include <array>

namespace PR {

constexpr int COEFFS_N = 3; // Has to be 3 all the time!
struct _SpectralUpsamplerInternal {
	uint32 Resolution;
	float* Scale;
	float* Data;
};

SpectralUpsampler::SpectralUpsampler(Serializer& serializer)
	: mInternal(new _SpectralUpsamplerInternal())
{
	mInternal->Resolution = 0;
	mInternal->Scale	  = nullptr;
	mInternal->Data		  = nullptr;

	char header[4];
	serializer.readRaw(reinterpret_cast<uint8*>(header), 4 * sizeof(char));

	if (memcmp(header, "SPEC", 4) != 0)
		throw std::runtime_error("Given spectral coefficients file is invalid");

	serializer.read(mInternal->Resolution);

	const size_t size_scale = mInternal->Resolution;
	const size_t size_data	= size_scale * size_scale * size_scale * 3 * COEFFS_N;

	mInternal->Scale = new float[size_scale];
	serializer.readRaw(reinterpret_cast<uint8*>(mInternal->Scale), size_scale * sizeof(float));

	mInternal->Data = new float[size_data];
	serializer.readRaw(reinterpret_cast<uint8*>(mInternal->Data), size_data * sizeof(float));
}

SpectralUpsampler::~SpectralUpsampler()
{
	if (mInternal->Scale)
		delete[] mInternal->Scale;
	if (mInternal->Data)
		delete[] mInternal->Data;
}

static int find_interval(const float* values, int size_, float x)
{
	int left		  = 0;
	int last_interval = size_ - 2;
	int size		  = last_interval;

	while (size > 0) {
		int half   = size >> 1;
		int middle = left + half + 1;

		if (values[middle] < x) {
			left = middle;
			size -= half + 1;
		} else {
			size = half;
		}
	}

	return std::min(left, last_interval);
}

// Note: Due to std::fma and IEC 60559 the case (+-)Inf*0 is NaN and we can not use (+-)Inf for the c terms
constexpr float EPS	   = 0.0001f;
constexpr float ZERO_A = 0;
constexpr float ZERO_B = 0;
constexpr float ZERO_C = -500.0f; // -500 is also possible and is closer to zero, but -50 is sufficient for floats
constexpr float ONE_A  = 0;
constexpr float ONE_B  = 0;
constexpr float ONE_C  = 5000000.0f;

inline void convert(_SpectralUpsamplerInternal* internal, const std::array<float, 3>& rgb, std::array<float, COEFFS_N>& coeffs)
{
	const uint32 res = internal->Resolution;
	const uint32 dx	 = COEFFS_N;
	const uint32 dy	 = COEFFS_N * res;
	const uint32 dz	 = COEFFS_N * res * res;

	const float* a_scale = internal->Scale;
	const float* a_data	 = internal->Data;

	// Handle special case when rgb is zero
	if (rgb[0] <= EPS && rgb[1] <= EPS && rgb[2] <= EPS) {
		coeffs[0] = ZERO_A;
		coeffs[1] = ZERO_B;
		coeffs[2] = ZERO_C;
		return;
	} else if (1 - rgb[0] <= EPS && 1 - rgb[1] <= EPS && 1 - rgb[2] <= EPS) {
		coeffs[0] = ONE_A;
		coeffs[1] = ONE_B;
		coeffs[2] = ONE_C;
		return;
	}

	// Determine largest entry
	int largest_entry = 0;
	PR_UNROLL_LOOP(2)
	for (int j = 1; j < 3; ++j)
		if (rgb[largest_entry] <= rgb[j])
			largest_entry = j;

	// Rescale
	float z		= rgb[largest_entry];
	float scale = (res - 1) / z;
	float x		= rgb[(largest_entry + 1) % 3] * scale;
	float y		= rgb[(largest_entry + 2) % 3] * scale;

	// Bilinearly interpolate
	uint32 xi  = std::min((uint32)x, res - 2);
	uint32 yi  = std::min((uint32)y, res - 2);
	uint32 zi  = find_interval(a_scale, res, z);
	uint32 off = (((largest_entry * res + zi) * res + yi) * res + xi) * COEFFS_N;

	float x1 = x - xi;
	float x0 = 1.0f - x1;
	float y1 = y - yi;
	float y0 = 1.0f - y1;
	float z1 = (z - a_scale[zi]) / (a_scale[zi + 1] - a_scale[zi]);
	float z0 = 1.0f - z1;

	// Lookup
	PR_UNROLL_LOOP(COEFFS_N)
	for (int j = 0; j < COEFFS_N; ++j) {
		coeffs[j] = ((a_data[off] * x0
					  + a_data[off + dx] * x1)
						 * y0
					 + (a_data[off + dy] * x0
						+ a_data[off + dy + dx] * x1)
						   * y1)
						* z0
					+ ((a_data[off + dz] * x0
						+ a_data[off + dz + dx] * x1)
						   * y0
					   + (a_data[off + dz + dy] * x0
						  + a_data[off + dz + dy + dx] * x1)
							 * y1)
						  * z1;
		++off;
	}
}

void SpectralUpsampler::prepare(const float* PR_RESTRICT r, const float* PR_RESTRICT g, const float* PR_RESTRICT b,
								float* PR_RESTRICT out_a, float* PR_RESTRICT out_b, float* PR_RESTRICT out_c, size_t elems)
{
	PR_ASSERT(mInternal->Data, "Expected valid spectral mapper");

	PR_OPT_LOOP
	for (size_t i = 0; i < elems; ++i) {
		std::array<float, COEFFS_N> coeffs;
		convert(mInternal.get(), { r[i], g[i], b[i] }, coeffs);

		out_a[i] = coeffs[0];
		out_b[i] = coeffs[1];
		out_c[i] = coeffs[2];
	}
}

void SpectralUpsampler::prepare(const float* PR_RESTRICT rgb, float* PR_RESTRICT out, size_t elems)
{
	PR_ASSERT(mInternal->Data, "Expected valid spectral mapper");

	PR_OPT_LOOP
	for (size_t i = 0; i < elems; ++i) {
		std::array<float, COEFFS_N> coeffs;
		convert(mInternal.get(), { rgb[3 * i + 0], rgb[3 * i + 1], rgb[3 * i + 2] }, coeffs);

		out[3 * i + 0] = coeffs[0];
		out[3 * i + 1] = coeffs[1];
		out[3 * i + 2] = coeffs[2];
	}
}
} // namespace PR