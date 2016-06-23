#include "xyzconvert.cl"

float3 gamma(float3 rgb)
{
	return (float3)(
		(rgb.x <= 0.0031308f) ? 12.92f*rgb.x : (1.055f*powr(rgb.x, 0.4166666f) - 0.055f),
		(rgb.y <= 0.0031308f) ? 12.92f*rgb.y : (1.055f*powr(rgb.y, 0.4166666f) - 0.055f),
		(rgb.z <= 0.0031308f) ? 12.92f*rgb.z : (1.055f*powr(rgb.z, 0.4166666f) - 0.055f));
} 

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

__kernel void m_srgb(__global float* specs, __global float* rgb)
{
	size_t id = get_global_id(0);

	float3 color = xyz_to_srgb(spec_to_xyz(&specs[id*SAMPLING_COUNT]));
	rgb[id*3] = color.x;
	rgb[id*3 + 1] = color.y;
	rgb[id*3 + 2] = color.z;
}

__kernel void m_srgb_gamma(__global float* specs, __global float* rgb)
{
	size_t id = get_global_id(0);

	float3 color = xyz_to_srgb(spec_to_xyz(&specs[id*SAMPLING_COUNT]));
	color = gamma(color);
	rgb[id*3] = color.x;
	rgb[id*3 + 1] = color.y;
	rgb[id*3 + 2] = color.z;
}

__kernel void m_srgb_byte(__global float* specs, __global uchar* rgb)
{
	size_t id = get_global_id(0);

	float3 color = xyz_to_srgb(spec_to_xyz(&specs[id*SAMPLING_COUNT]));
	rgb[id*3] = (uchar)(clamp(color.x, 0.0f, 1.0f) * 255);
	rgb[id*3 + 1] = (uchar)(clamp(color.y, 0.0f, 1.0f) * 255);
	rgb[id*3 + 2] = (uchar)(clamp(color.z, 0.0f, 1.0f) * 255);
}

__kernel void m_srgb_gamma_byte(__global float* specs, __global uchar* rgb)
{
	size_t id = get_global_id(0);

	float3 color = xyz_to_srgb(spec_to_xyz(&specs[id*SAMPLING_COUNT]));
	color = gamma(color);
	rgb[id*3] = (uchar)(clamp(color.x, 0.0f, 1.0f) * 255);
	rgb[id*3 + 1] = (uchar)(clamp(color.y, 0.0f, 1.0f) * 255);
	rgb[id*3 + 2] = (uchar)(clamp(color.z, 0.0f, 1.0f) * 255);
}