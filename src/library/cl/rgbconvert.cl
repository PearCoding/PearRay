#include "xyzconvert.cl"

float3 xyz_to_srgb(float3 rgb)
{
	float X2 = 0.9531874f*rgb.x - 0.0265906f*rgb.y + 0.0238731f*rgb.z;
	float Y2 = -0.0382467f*rgb.x + 1.0288406f*rgb.y + 0.0094060f*rgb.z;
	float Z2 = 0.0026068f*rgb.x - 0.0030332f*rgb.y + 1.0892565f*rgb.z;

	return (float3)(
		3.240479f * X2 - 1.537150f * Y2 - 0.498535f * Z2,
		-0.969256f * X2 + 1.875991f * Y2 + 0.041556f * Z2,
		0.055648f * X2 - 0.204043f * Y2 + 1.057311f * Z2);
}

__kernel void k_srgb(__global const float* specs, __global float* rgb, ulong off)
{
	size_t id = get_global_id(0);

	const float3 color = xyz_to_srgb(spec_to_xyz(&specs[id*SAMPLING_COUNT]));
	id += off;
	rgb[id*3] = color.x;
	rgb[id*3 + 1] = color.y;
	rgb[id*3 + 2] = color.z;
}

__kernel void k_lum(__global const float* specs, __global float* rgb, ulong off)
{
	size_t id = get_global_id(0);
	const float3 color = xyz_to_srgb(spec_to_xyz(&specs[id*SAMPLING_COUNT]));
	const float lum = luminance(color.x, color.y, color.z);

	id += off;
	rgb[id*3] = lum;
	rgb[id*3 + 1] = lum;
	rgb[id*3 + 2] = lum;
}