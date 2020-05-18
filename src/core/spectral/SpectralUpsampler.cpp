#include "SpectralUpsampler.h"
#include "serialization/Serializer.h"

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
	serializer.readRaw(reinterpret_cast<uint8*>(header), 4, sizeof(char));

	if (memcmp(header, "SPEC", 4) != 0)
		throw std::runtime_error("Given spectral coefficients file is invalid");

	serializer.read(mInternal->Resolution);

	const size_t size_scale = mInternal->Resolution;
	const size_t size_data	= mInternal->Resolution * mInternal->Resolution * mInternal->Resolution * 3 * COEFFS_N;

	mInternal->Scale = new float[size_scale];
	serializer.readRaw(reinterpret_cast<uint8*>(mInternal->Scale), size_scale, sizeof(float));

	mInternal->Data = new float[size_data];
	serializer.readRaw(reinterpret_cast<uint8*>(mInternal->Data), size_data, sizeof(float));
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
		int half   = size >> 1,
			middle = left + half + 1;

		if (values[middle] < x) {
			left = middle;
			size -= half + 1;
		} else {
			size = half;
		}
	}

	return std::min(left, last_interval);
}

void SpectralUpsampler::prepare(const float* r, const float* g, const float* b, float* out_a, float* out_b, float* out_c, size_t elems)
{
	PR_ASSERT(mInternal->Data, "Expected valid spectral mapper");
	const float* arr[3] = { r, g, b };
	const uint32 res	= mInternal->Resolution - 1;
	const uint32 dx		= COEFFS_N;
	const uint32 dy		= COEFFS_N * res;
	const uint32 dz		= COEFFS_N * res * res;

	const float* a_scale = mInternal->Scale;
	const float* a_data	 = mInternal->Data;

	float coeffs[COEFFS_N];
	PR_OPT_LOOP
	for (size_t i = 0; i < elems; ++i) {
		// Determine largest entry
		int largest_entry = 0;
		PR_UNROLL_LOOP(2)
		for (int j = 1; j < 3; ++j)
			if (arr[largest_entry][i] < arr[j][i])
				largest_entry = j;

		// Rescale
		float z		= arr[largest_entry][i];
		float scale = (res - 1) / z;
		float x		= arr[(largest_entry + 1) % 3][i] * scale;
		float y		= arr[(largest_entry + 2) % 3][i] * scale;

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

		out_a[i] = coeffs[0];
		out_b[i] = coeffs[1];
		out_c[i] = coeffs[2];
	}
}
} // namespace PR