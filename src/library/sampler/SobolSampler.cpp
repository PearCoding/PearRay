#include "SobolSampler.h"

namespace PR {
/* Based on http://web.maths.unsw.edu.au/~fkuo/sobol/index.html
 * Direction vectors: new-joe-kuo-7.21201
 */

#define SOBOL_MAX_DIM (101)
#define SOBOL_MAX_M_DIM (9)
static uint32 sobol_s[SOBOL_MAX_DIM]
	= { /* First dim is copy of second in our case */
		1, 1, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9,
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
	  };

static uint32 sobol_m[SOBOL_MAX_DIM][SOBOL_MAX_M_DIM]
	= {
		  { 1, 0, 0, 0, 0, 0, 0, 0, 0 },
		  { 1, 0, 0, 0, 0, 0, 0, 0, 0 },
		  { 1, 3, 0, 0, 0, 0, 0, 0, 0 },
		  { 1, 3, 1, 0, 0, 0, 0, 0, 0 },
		  { 1, 1, 1, 0, 0, 0, 0, 0, 0 },
		  { 1, 1, 3, 3, 0, 0, 0, 0, 0 },
		  { 1, 3, 5, 13, 0, 0, 0, 0, 0 },
		  { 1, 1, 5, 5, 17, 0, 0, 0, 0 },
		  { 1, 1, 5, 5, 5, 0, 0, 0, 0 },
		  { 1, 1, 7, 11, 19, 0, 0, 0, 0 },
		  { 1, 1, 5, 1, 1, 0, 0, 0, 0 },
		  { 1, 1, 1, 3, 11, 0, 0, 0, 0 },
		  { 1, 3, 5, 5, 31, 0, 0, 0, 0 },
		  { 1, 3, 3, 9, 7, 49, 0, 0, 0 },
		  { 1, 1, 1, 15, 23, 17, 0, 0, 0 },
		  { 1, 3, 1, 13, 27, 49, 0, 0, 0 },
		  { 1, 1, 1, 15, 7, 5, 0, 0, 0 },
		  { 1, 3, 7, 7, 19, 13, 0, 0, 0 },
		  { 1, 1, 5, 5, 19, 59, 0, 0, 0 },
		  { 1, 3, 7, 11, 23, 7, 121, 0, 0 },
		  { 1, 3, 5, 13, 9, 19, 113, 0, 0 },
		  { 1, 1, 7, 15, 3, 53, 87, 0, 0 },
		  { 1, 3, 7, 7, 25, 33, 41, 0, 0 },
		  { 1, 3, 1, 9, 15, 47, 85, 0, 0 },
		  { 1, 1, 7, 3, 5, 25, 37, 0, 0 },
		  { 1, 3, 7, 9, 31, 11, 27, 0, 0 },
		  { 1, 1, 7, 15, 1, 17, 121, 0, 0 },
		  { 1, 3, 7, 13, 17, 49, 5, 0, 0 },
		  { 1, 3, 5, 9, 7, 9, 31, 0, 0 },
		  { 1, 3, 1, 9, 7, 29, 15, 0, 0 },
		  { 1, 3, 1, 1, 23, 43, 37, 0, 0 },
		  { 1, 3, 7, 5, 3, 43, 95, 0, 0 },
		  { 1, 1, 7, 9, 31, 25, 91, 0, 0 },
		  { 1, 3, 1, 11, 15, 17, 91, 0, 0 },
		  { 1, 1, 1, 1, 21, 29, 71, 0, 0 },
		  { 1, 1, 5, 13, 27, 3, 27, 0, 0 },
		  { 1, 3, 1, 9, 31, 25, 3, 0, 0 },
		  { 1, 3, 3, 1, 25, 23, 51, 45, 0 },
		  { 1, 1, 5, 13, 9, 25, 17, 227, 0 },
		  { 1, 1, 1, 1, 15, 9, 91, 45, 0 },
		  { 1, 1, 7, 7, 9, 35, 87, 31, 0 },
		  { 1, 1, 5, 13, 29, 33, 13, 119, 0 },
		  { 1, 3, 5, 3, 17, 9, 73, 85, 0 },
		  { 1, 3, 7, 5, 13, 33, 11, 17, 0 },
		  { 1, 1, 3, 13, 29, 25, 15, 75, 0 },
		  { 1, 3, 7, 9, 3, 11, 11, 59, 0 },
		  { 1, 3, 1, 13, 5, 27, 19, 71, 0 },
		  { 1, 1, 7, 3, 13, 41, 79, 137, 0 },
		  { 1, 3, 3, 7, 9, 41, 89, 63, 0 },
		  { 1, 1, 1, 5, 29, 57, 53, 103, 0 },
		  { 1, 1, 3, 9, 3, 41, 71, 119, 0 },
		  { 1, 3, 5, 5, 3, 1, 39, 185, 0 },
		  { 1, 3, 3, 15, 23, 3, 43, 95, 0 },
		  { 1, 1, 3, 7, 1, 35, 19, 19, 127 },
		  { 1, 1, 3, 9, 9, 31, 9, 157, 105 },
		  { 1, 3, 1, 1, 7, 49, 13, 125, 471 },
		  { 1, 3, 3, 1, 17, 25, 111, 73, 41 },
		  { 1, 3, 5, 11, 3, 61, 117, 93, 325 },
		  { 1, 1, 7, 15, 21, 63, 103, 145, 61 },
		  { 1, 3, 7, 7, 27, 49, 101, 227, 55 },
		  { 1, 1, 7, 5, 1, 1, 33, 225, 501 },
		  { 1, 3, 5, 5, 1, 17, 113, 149, 71 },
		  { 1, 1, 5, 9, 27, 9, 97, 7, 35 },
		  { 1, 3, 1, 7, 13, 39, 49, 213, 483 },
		  { 1, 3, 3, 11, 13, 47, 43, 89, 329 },
		  { 1, 3, 1, 1, 19, 61, 97, 205, 285 },
		  { 1, 1, 3, 11, 11, 3, 105, 151, 469 },
		  { 1, 1, 1, 1, 5, 39, 89, 137, 409 },
		  { 1, 1, 3, 11, 23, 39, 85, 251, 141 },
		  { 1, 1, 3, 5, 23, 23, 59, 111, 19 },
		  { 1, 3, 5, 9, 19, 63, 1, 251, 305 },
		  { 1, 3, 1, 1, 3, 35, 65, 153, 387 },
		  { 1, 1, 5, 13, 21, 45, 37, 255, 295 },
		  { 1, 1, 5, 1, 21, 5, 39, 143, 293 },
		  { 1, 1, 1, 5, 21, 45, 91, 211, 347 },
		  { 1, 3, 5, 11, 17, 21, 73, 185, 259 },
		  { 1, 3, 1, 11, 29, 25, 85, 103, 97 },
		  { 1, 3, 5, 5, 27, 37, 43, 243, 257 },
		  { 1, 3, 5, 11, 31, 63, 105, 41, 229 },
		  { 1, 3, 3, 15, 21, 55, 29, 39, 279 },
		  { 1, 3, 5, 7, 1, 47, 97, 15, 173 },
		  { 1, 1, 5, 7, 21, 55, 29, 93, 97 },
		  { 1, 3, 3, 3, 7, 29, 27, 179, 105 },
		  { 1, 1, 3, 3, 1, 43, 37, 221, 259 },
		  { 1, 1, 7, 9, 31, 25, 1, 47, 71 },
		  { 1, 3, 7, 7, 7, 25, 37, 1, 487 },
		  { 1, 3, 1, 1, 31, 43, 25, 117, 147 },
		  { 1, 3, 7, 3, 27, 7, 103, 241, 173 },
		  { 1, 1, 5, 1, 27, 25, 91, 189, 53 },
		  { 1, 1, 3, 3, 29, 39, 79, 125, 281 },
		  { 1, 1, 5, 11, 13, 57, 15, 219, 407 },
		  { 1, 3, 3, 5, 15, 9, 65, 111, 1 },
		  { 1, 1, 5, 3, 31, 45, 17, 227, 67 },
		  { 1, 1, 3, 1, 31, 59, 63, 205, 27 },
		  { 1, 1, 1, 9, 27, 9, 49, 187, 451 },
		  { 1, 1, 5, 15, 11, 43, 93, 49, 129 },
		  { 1, 3, 7, 7, 5, 63, 31, 239, 11 },
		  { 1, 1, 1, 11, 7, 59, 37, 157, 505 },
		  { 1, 3, 5, 9, 3, 41, 87, 229, 269 },
		  { 1, 1, 5, 3, 25, 11, 69, 125, 35 },
		  { 1, 3, 5, 13, 11, 53, 47, 21, 79 }
	  };

HaltonQMCSampler::HaltonQMCSampler(uint32 samples,
								   uint32 baseX, uint32 baseY, uint32 baseZ)
	: Sampler()
	, mSamples(samples)
	, mBaseXSamples(samples)
	, mBaseYSamples(samples)
	, mBaseZSamples(samples)
	, mAdaptive(adaptive)
	, mBaseX(baseX)
	, mBaseY(baseY)
	, mBaseZ(baseZ)
{
	PR_ASSERT(samples > 0, "Given sample count has to be greater than 0");

	for (uint32 i = 0; i < samples; ++i) {
		mBaseXSamples[i] = halton(i, mBaseX);
		mBaseYSamples[i] = halton(i, mBaseY);
		mBaseZSamples[i] = halton(i, mBaseZ);
	}
}

HaltonQMCSampler::~HaltonQMCSampler()
{
}

float HaltonQMCSampler::generate1D(uint32 index)
{
	if (index < mSamples)
		return mBaseXSamples[index % mSamples];
	else // To allow adaptive methods with higher samples
		return halton(index, mBaseX);
}

Eigen::Vector2f HaltonQMCSampler::generate2D(uint32 index)
{
	if (index < mSamples)
		return Eigen::Vector2f(mBaseXSamples[index % mSamples], mBaseYSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Eigen::Vector2f(halton(index, mBaseX),
							   halton(index, mBaseY));
}

Eigen::Vector3f HaltonQMCSampler::generate3D(uint32 index)
{
	if (index < mSamples)
		return Eigen::Vector3f(mBaseXSamples[index % mSamples],
							   mBaseYSamples[index % mSamples],
							   mBaseZSamples[index % mSamples]);
	else // To allow adaptive methods with higher samples
		return Eigen::Vector3f(halton(index, mBaseX),
							   halton(index, mBaseY),
							   halton(index, mBaseZ));
}

void HaltonQMCSampler::generate1Dv(uint32 index, vfloat& s1)
{
	if (index * PR_SIMD_BANDWIDTH + PR_SIMD_BANDWIDTH - 1 < mSamples)
		s1 = simdpp::load(&mBaseXSamples[index * PR_SIMD_BANDWIDTH]);
	else // To allow adaptive methods with higher samples
		s1 = haltonv(index, mBaseX);
}

void HaltonQMCSampler::generate2Dv(uint32 index, vfloat& s1, vfloat& s1)
{
	if (index * PR_SIMD_BANDWIDTH + PR_SIMD_BANDWIDTH - 1 < mSamples) {
		s1 = simdpp::load(&mBaseXSamples[index * PR_SIMD_BANDWIDTH]);
		s2 = simdpp::load(&mBaseYSamples[index * PR_SIMD_BANDWIDTH]);
	} else { // To allow adaptive methods with higher samples
		s1 = haltonv(index, mBaseX);
		s2 = haltonv(index, mBaseY);
	}
}

void HaltonQMCSampler::generate3Dv(uint32 index, vfloat& s1, vfloat& s1, vfloat& s1)
{
	if (index * PR_SIMD_BANDWIDTH + PR_SIMD_BANDWIDTH - 1 < mSamples) {
		s1 = simdpp::load(&mBaseXSamples[index * PR_SIMD_BANDWIDTH]);
		s2 = simdpp::load(&mBaseYSamples[index * PR_SIMD_BANDWIDTH]);
		s3 = simdpp::load(&mBaseZSamples[index * PR_SIMD_BANDWIDTH]);
	} else { // To allow adaptive methods with higher samples
		s1 = haltonv(index, mBaseX);
		s2 = haltonv(index, mBaseY);
		s3 = haltonv(index, mBaseZ);
	}
}
} // namespace PR
