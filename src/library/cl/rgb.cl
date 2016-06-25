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

//Really?
__kernel void k_to_byte(__global const float* in, __global uchar* out, const ulong size)
{ 
	size_t id = get_global_id(0);
	out[id] = (uchar)clamp(255*in[id],0.0f,255.0f);
}