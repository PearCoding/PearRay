// SAMPLING_COUNT should be defined extern
#define ILL_SCALE (4.94382f)
#define N (105.654099f)

#include "xyztable.cl"

float3 spec_to_xyz(__global const float* specs)
{
	float3 col = 0;

#pragma unroll SAMPLING_COUNT
	for(int i = 0; i < SAMPLING_COUNT; ++i)
	{
		col += specs[i] * (float3)(NM_TO_X[i], NM_TO_Y[i], NM_TO_Z[i]);
	}
	
	return col * (ILL_SCALE / N);
}

__kernel void k_xyz(__global const float* specs, __global float* rgb, ulong off)
{
	size_t id = get_global_id(0);

	const float3 color = spec_to_xyz(&specs[id*SAMPLING_COUNT]);
	id += off;
	rgb[id*3] = color.x;
	rgb[id*3 + 1] = color.y;
	rgb[id*3 + 2] = color.z;
}

__kernel void k_xyz_norm(__global const float* specs, __global float* rgb, ulong off)
{
	size_t id = get_global_id(0);

	const float3 color = spec_to_xyz(&specs[id*SAMPLING_COUNT]);
	const float m = 1 / (color.x + color.y + color.z + 0.000001f);

	id += off;
	rgb[id*3] = color.x * m;
	rgb[id*3 + 1] = color.y * m;
	rgb[id*3 + 2] = 1 - (color.x - color.y) * m;
}