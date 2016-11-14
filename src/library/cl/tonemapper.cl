R"(
// THIS FILE CAN'T BE USED BY EXTERNAL OPENCL PARSERS.
// DON'T REMOVE THE TAG AT THE TOP AND BOTTOM LINE.
// THIS FILE IS TO BE INCLUDED BY C++!
// SAMPLING_COUNT should be defined extern
#define ILL_SCALE (SAMPLING_COUNT + 1)
#define N (21.371327f * ILL_SCALE)

// Utils
float luminance(float r, float g, float b)
{
	return 0.2126f*r + 0.7152f*g + 0.0722f*b;
}

__kernel void k_gamma(__global float* rgb, const ulong size)
{
	size_t id = get_global_id(0);
	const float val = rgb[id];
	rgb[id] = (val <= 0.0031308f) ? 12.92f*val : (1.055f*powr(val, 0.4166666f) - 0.055f);
} 

// XYZ
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

// sRGB
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

// Tone Mapping
__kernel void k_tone_reinhard_luminance_log(__global const float* in, __global float* out, ulong size, __local float* tmp)
{
    uint gid = get_global_id(0);
	uint grpid = get_group_id(0);
	uint tid = get_local_id(0);
	uint tsize = get_local_size(0);

	// Using reduction scheme
	float local_acc = 0;
	if(gid < size)
	{
		local_acc = log(0.000001f + luminance(in[gid*3],in[gid*3 + 1],in[gid*3 + 2]));
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	uint dist = tsize;
	while ( dist > 1 )
	{
		dist >>= 1;//dist/=2;
		if ( tid < dist )
		{
			local_acc += tmp[tid + dist];
			tmp[tid] = local_acc;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if ( tid == 0 )
	{
		out[grpid] = local_acc;
	}
}

__kernel void k_tone_reinhard_simple(__global float* rgb, ulong size, float ratio)
{
    uint gid = get_global_id(0);
	float L = luminance(rgb[gid*3],rgb[gid*3 + 1],rgb[gid*3 + 2]) * ratio;
	float Ld = 1 / (1 + L);
	rgb[gid*3] *= Ld;
	rgb[gid*3 + 1] *= Ld;
	rgb[gid*3 + 2] *= Ld;
}

__kernel void k_tone_clamp(__global float* rgb, ulong size)
{
	uint gid = get_global_id(0);
	rgb[gid*3] = min(abs(rgb[gid*3]), 1.0);
	rgb[gid*3+1] = min(abs(rgb[gid*3+1]), 1.0);
	rgb[gid*3+2] = min(abs(rgb[gid*3+2]), 1.0);
}

__kernel void k_tone_abs(__global float* rgb, ulong size)
{
	uint gid = get_global_id(0);
	rgb[gid*3] = abs(rgb[gid*3]);
	rgb[gid*3+1] = abs(rgb[gid*3+1]);
	rgb[gid*3+2] = abs(rgb[gid*3+2]);
}

__kernel void k_tone_positive(__global float* rgb, ulong size)
{
	uint gid = get_global_id(0);
	rgb[gid*3] = max(rgb[gid*3], 0.0);
	rgb[gid*3+1] = max(rgb[gid*3+1], 0.0);
	rgb[gid*3+2] = max(rgb[gid*3+2], 0.0);
}

__kernel void k_tone_negative(__global float* rgb, ulong size)
{
	uint gid = get_global_id(0);
	rgb[gid*3] = -min(rgb[gid*3], 0.0);
	rgb[gid*3+1] = -min(rgb[gid*3+1], 0.0);
	rgb[gid*3+2] = -min(rgb[gid*3+2], 0.0);
}

__kernel void k_tone_normalize(__global float* rgb, ulong size, float invMax)
{
	uint gid = get_global_id(0);
	rgb[gid*3] *= invMax;
	rgb[gid*3+1] *= invMax;
	rgb[gid*3+2] *= invMax;
}

// Assume normalized vector
__kernel void k_tone_spherical(__global float* rgb, ulong size)
{
	uint gid = get_global_id(0);
	float rho = 0.5 + 0.5 * atan2(rgb[gid*3+2], rgb[gid*3]) / 3.141592;
	float phi = 0.5 - asin(-rgb[gid*3+1]) / 3.141592;

	rgb[gid*3] = rho;
	rgb[gid*3+1] = phi;
	rgb[gid*3+2] = 0;
}

)";
