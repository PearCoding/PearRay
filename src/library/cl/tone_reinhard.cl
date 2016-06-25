#include "rgb.cl"

__kernel void k_tone_reinhard_luminance_log(__global const float* in, __global float* out, unsigned long size,
 __local float* tmp)
{
    unsigned int gid = get_global_id(0);
	unsigned int grpid = get_group_id(0);
	unsigned int tid = get_local_id(0);
	unsigned int tsize = get_local_size(0);

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

// No dodging-and-burning
__kernel void k_tone_reinhard_simple(__global float* rgb, unsigned long size, float ratio)
{
    unsigned int gid = get_global_id(0);
	float L = luminance(rgb[gid*3],rgb[gid*3 + 1],rgb[gid*3 + 2]) * ratio;
	float Ld = L/(1+L);
	rgb[gid*3] *= Ld;
	rgb[gid*3 + 1] *= Ld;
	rgb[gid*3 + 2] *= Ld;
}
