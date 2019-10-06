#pragma once

#include "Random.h"

namespace PR {
class PR_LIB Sampler {
public:
	virtual ~Sampler() = default;

	virtual float generate1D(uint32 index)			 = 0;
	virtual Vector2f generate2D(uint32 index) = 0;
	virtual Vector3f generate3D(uint32 index) = 0;

	virtual void generate1Dv(uint32 index, vfloat& s1)
	{
		PR_SIMD_ALIGN float r[PR_SIMD_BANDWIDTH];

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i)
			r[i] = generate1D(index * PR_SIMD_BANDWIDTH + i);

		s1 = simdpp::load(r);
	}

	virtual void generate2Dv(uint32 index, vfloat& s1, vfloat& s2)
	{
		PR_SIMD_ALIGN float r1[PR_SIMD_BANDWIDTH];
		PR_SIMD_ALIGN float r2[PR_SIMD_BANDWIDTH];

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
			auto v = generate2D(index * PR_SIMD_BANDWIDTH + i);
			r1[i]  = v(0);
			r2[i]  = v(1);
		}

		s1 = simdpp::load(r1);
		s2 = simdpp::load(r2);
	}

	virtual void generate3Dv(uint32 index, vfloat& s1, vfloat& s2, vfloat& s3)
	{
		PR_SIMD_ALIGN float r1[PR_SIMD_BANDWIDTH];
		PR_SIMD_ALIGN float r2[PR_SIMD_BANDWIDTH];
		PR_SIMD_ALIGN float r3[PR_SIMD_BANDWIDTH];

		for (size_t i = 0; i < PR_SIMD_BANDWIDTH; ++i) {
			auto v = generate3D(index * PR_SIMD_BANDWIDTH + i);
			r1[i]  = v(0);
			r2[i]  = v(1);
			r3[i]  = v(2);
		}

		s1 = simdpp::load(r1);
		s2 = simdpp::load(r2);
		s3 = simdpp::load(r3);
	}
};
} // namespace PR
