// SAMPLING_COUNT should be defined extern
#define ILL_SCALE (4.94382f)
#define N (105.654099f)

constant float NM_TO_X[SAMPLING_COUNT] = {
	0.00013f, 0.00023f, 0.00041f, 0.00074f, 0.00137f, 0.00223f, 0.00424f, 0.00765f, 0.01431f, 0.02319f,
	0.04351f, 0.07763f, 0.13438f, 0.21477f, 0.2839f, 0.3285f, 0.34828f, 0.34806f, 0.3362f, 0.3187f,
	0.2908f, 0.2511f, 0.19536f, 0.1421f, 0.09564f, 0.05795f, 0.03201f, 0.0147f, 0.0049f, 0.0024f,
	0.0093f, 0.0291f, 0.06327f, 0.1096f, 0.1655f, 0.22575f, 0.2904f, 0.3597f, 0.43345f, 0.51205f,
	0.5945f, 0.6784f, 0.7621f, 0.8425f, 0.9163f, 0.9786f, 1.0263f, 1.0567f, 1.0622f, 1.0456f,
	1.0026f, 0.9384f, 0.85445f, 0.7514f, 0.6424f, 0.5419f, 0.4479f, 0.3608f, 0.2835f, 0.2187f,
	0.1649f, 0.1212f, 0.0874f, 0.0636f, 0.04677f, 0.0329f, 0.0227f, 0.01584f, 0.01136f, 0.00811f,
	0.00579f, 0.00411f, 0.00289f, 0.00205f, 0.00144f, 0.001f, 0.00069f, 0.00048f, 0.00033f, 0.00023f,
	0.00017f, 0.00012f, 8e-05f, 6e-05f, 4.1e-05f, 2.9e-05f, 2e-05f, 1.4e-05f, 1e-05f
};

constant float NM_TO_Y[SAMPLING_COUNT] = {
	0, 0, 1e-05f, 2e-05f, 4e-05f, 6e-05f, 0.00012f, 0.00022f, 0.0004f, 0.00064f,
	0.0012f, 0.00218f, 0.004f, 0.0073f, 0.0116f, 0.01684f, 0.023f, 0.0298f, 0.038f, 0.048f,
	0.06f, 0.0739f, 0.09098f, 0.1126f, 0.13902f, 0.1693f, 0.20802f, 0.2586f, 0.323f, 0.4073f,
	0.503f, 0.6082f, 0.71f, 0.7932f, 0.862f, 0.91485f, 0.954f, 0.9803f, 0.99495f, 1,
	0.995f, 0.9786f, 0.952f, 0.9154f, 0.87f, 0.8163f, 0.757f, 0.6949f, 0.631f, 0.5668f,
	0.503f, 0.4412f, 0.381f, 0.321f, 0.265f, 0.217f, 0.175f, 0.1382f, 0.107f, 0.0816f,
	0.061f, 0.04458f, 0.032f, 0.0232f, 0.017f, 0.01192f, 0.00821f, 0.00573f, 0.0041f, 0.00293f,
	0.00209f, 0.00105f, 0.00105f, 0.00074f, 0.00052f, 0.00036f, 0.00025f, 0.00017f, 0.00012f, 8e-05f,
	6e-05f, 4e-05f, 3e-05f, 2e-05f, 1.4e-05f, 1e-05f, 7e-06f, 5e-06f, 3e-06f
};

constant float NM_TO_Z[SAMPLING_COUNT] = {
	0.00061f, 0.00108f, 0.00195f, 0.00349f, 0.00645f, 0.01055f, 0.02005f, 0.03621f, 0.06785f, 0.1102f,
	0.2074f, 0.3713f, 0.6456f, 1.03905f, 1.3856f, 1.62296f, 1.74706f, 1.7826f, 1.77211f, 1.7441f,
	1.6692f, 1.5281f, 1.28764f, 1.0419f, 0.81295f, 0.6162f, 0.46518f, 0.3533f, 0.272f, 0.2123f,
	0.1582f, 0.1117f, 0.07825f, 0.05725f, 0.04216f, 0.02984f, 0.0203f, 0.0134f, 0.00875f, 0.00575f,
	0.0039f, 0.00275f, 0.0021f, 0.0018f, 0.00165f, 0.0014f, 0.0011f, 0.001f, 0.0008f, 0.0006f,
	0.00034f, 0.00024f, 0.00019f, 0.0001f, 5e-05f, 3e-05f, 2e-05f, 1e-05f, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0
};

float3 spec_to_xyz(global float* specs)
{
	float3 col = 0;

#pragma unroll 
	for(int i = 0; i < SAMPLING_COUNT; ++i)
	{
		const float val = specs[i];
		col += val * (float3)(NM_TO_X[i], NM_TO_Y[i], NM_TO_Z[i]);
	}

	return col * ILL_SCALE / N;
}

__kernel void m_xyz(__global float* specs, __global float* rgb)
{
	size_t id = get_global_id(0);

	float3 color = spec_to_xyz(&specs[id*SAMPLING_COUNT]);
	rgb[id*3] = color.x;
	rgb[id*3 + 1] = color.y;
	rgb[id*3 + 2] = color.z;
}

__kernel void m_xyz_byte(__global float* specs, __global uchar* rgb)
{
	size_t id = get_global_id(0);

	float3 color = spec_to_xyz(&specs[id*SAMPLING_COUNT]);
	rgb[id*3] = (uchar)(color.x * 255);
	rgb[id*3 + 1] = (uchar)(color.y * 255);
	rgb[id*3 + 2] = (uchar)(color.z * 255);
}

__kernel void m_xyz_norm(__global float* specs, __global float* rgb)
{
	size_t id = get_global_id(0);

	float3 color = spec_to_xyz(&specs[id*SAMPLING_COUNT]);
	float m = color.x + color.y + color.z;
	if (m != 0)
	{
		rgb[id*3] = color.x / m;
		rgb[id*3 + 1] = color.y / m;
		rgb[id*3 + 2] = 1 - (color.x - color.y)/m;
	}
	else
	{
		rgb[id*3] = 0;
		rgb[id*3 + 1] = 0;
		rgb[id*3 + 2] = 1;
	}
}

__kernel void m_xyz_norm_byte(__global float* specs, __global uchar* rgb)
{
	size_t id = get_global_id(0);

	float3 color = spec_to_xyz(&specs[id*SAMPLING_COUNT]);
	float m = color.x + color.y + color.z;
	if (m != 0)
	{
		rgb[id*3] = (uchar)(255*color.x / m);
		rgb[id*3 + 1] = (uchar)(255*color.y / m);
		rgb[id*3 + 2] = (uchar)(255*(1 - (color.x - color.y)/m));
	}
	else
	{
		rgb[id*3] = 0;
		rgb[id*3 + 1] = 0;
		rgb[id*3 + 2] = 255;
	}
}