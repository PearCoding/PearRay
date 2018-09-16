#include "ispc/math.isph"

export void sample_lambert(
	uniform const float in_rnd1[], uniform const float in_rnd2[],
	uniform float out_pdf_s_forward[], uniform float out_pdf_s_backward[],
	uniform float out_dir_x[], uniform float out_dir_y[], uniform float out_dir_z[],
    uniform int size) {

	for(uniform int i = 0; i < size; i += programCount) {
		int index = i + programIndex;

		float pdf_s, dx, dy, dz;
		sample_cos_hemi(in_rnd1[index], in_rnd2[index],
			pdf_s, dx, dy, dz);

		out_pdf_s_forward[index] = pdf_s;
		out_pdf_s_backward[index] = pdf_s;
		out_dir_x[index] = dx;
		out_dir_y[index] = dy;
		out_dir_z[index] = dz;
	}
}