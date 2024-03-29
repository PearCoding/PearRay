#include "Test.h"
#include "math/Microfacet.h"
#include "math/Scattering.h"

using namespace PR;

#ifdef PR_DEBUG
#ifndef PR_TEST_VERBOSE
#define PR_TEST_VERBOSE
#endif
#endif

PR_BEGIN_TESTCASE(BRDF)
PR_TEST("Scattering")
{
	auto N = Vector3f(0, 1, 0);
	auto V = Vector3f(1, -1, 0).normalized();

	auto R = Scattering::reflect(-V, N);

	PR_CHECK_NEARLY_EQ(R, Vector3f(V(0), -V(1), V(2)));
}

// FIXME!!
/*PR_TEST("Refraction")
{
	auto N = Vector3f(0, 1, 0);
	auto V = Vector3f(1, -1, 0).normalized();

	auto R = Scattering::refract(0.9f, -V.dot(N), -V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0.636396f, -0.771362f, 0));
}*/

PR_TEST("Blinn NDF")
{
	constexpr uint32 MAX_SAMPLES = 16;
	constexpr float SAMPLE_STEP  = 1.0f / MAX_SAMPLES;

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 0.5\n";
#endif
	float alpha = 0.5f;
	float sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES; ++i) {
		float d = Microfacet::ndf_blinn(i * SAMPLE_STEP, alpha);
#ifdef PR_TEST_VERBOSE
		std::cout << "[" << i << "|" << i * SAMPLE_STEP << "-> " << d << "] ";
#endif
		sum += d;
	}
	sum /= MAX_SAMPLES - 1;
	PR_CHECK_GREAT_EQ(sum, 0);
	PR_CHECK_LESS_EQ(sum, 1);

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 0.001\n";
#endif
	alpha = 0.001f;
	sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES; ++i) {
		float d = Microfacet::ndf_blinn(i * SAMPLE_STEP, alpha);
#ifdef PR_TEST_VERBOSE
		std::cout << "[" << i << "|" << i * SAMPLE_STEP << "-> " << d << "] ";
#endif
		sum += d;
	}
	sum /= MAX_SAMPLES - 1;
	PR_CHECK_GREAT_EQ(sum, 0);
	PR_CHECK_LESS_EQ(sum, 1);

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 1\n";
#endif
	alpha = 1;
	sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES; ++i) {
		float d = Microfacet::ndf_blinn(i * SAMPLE_STEP, alpha);
#ifdef PR_TEST_VERBOSE
		std::cout << "[" << i << "|" << i * SAMPLE_STEP << "-> " << d << "] ";
#endif
		sum += d;
	}
	sum /= MAX_SAMPLES - 1;
	PR_CHECK_GREAT_EQ(sum, 0);
	PR_CHECK_LESS_EQ(sum, 1);

#ifdef PR_TEST_VERBOSE
	std::cout << std::endl;
#endif
}

PR_TEST("Beckmann NDF")
{
	constexpr uint32 MAX_SAMPLES = 16;
	constexpr float SAMPLE_STEP  = 1.0f / MAX_SAMPLES;

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 0.5\n";
#endif
	float alpha = 0.5f;
	float sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES; ++i) {
		float d = Microfacet::ndf_beckmann(i * SAMPLE_STEP, alpha);
#ifdef PR_TEST_VERBOSE
		std::cout << "[" << i << "|" << i * SAMPLE_STEP << "-> " << d << "] ";
#endif
		sum += d;
	}
	sum /= MAX_SAMPLES - 1;
	PR_CHECK_GREAT_EQ(sum, 0);
	PR_CHECK_LESS_EQ(sum, 1);

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 0.001\n";
#endif
	alpha = 0.001f;
	sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES; ++i) {
		float d = Microfacet::ndf_beckmann(i * SAMPLE_STEP, alpha);
#ifdef PR_TEST_VERBOSE
		std::cout << "[" << i << "|" << i * SAMPLE_STEP << "-> " << d << "] ";
#endif
		sum += d;
	}
	sum /= MAX_SAMPLES - 1;
	PR_CHECK_GREAT_EQ(sum, 0);
	PR_CHECK_LESS_EQ(sum, 1);

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 1\n";
#endif
	alpha = 1;
	sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES - 1; ++i) {
		float d = Microfacet::ndf_beckmann(i * SAMPLE_STEP, alpha);
#ifdef PR_TEST_VERBOSE
		std::cout << "[" << i << "|" << i * SAMPLE_STEP << "-> " << d << "] ";
#endif
		sum += d;
	}
	sum /= MAX_SAMPLES - 1;
	PR_CHECK_GREAT_EQ(sum, 0);
	PR_CHECK_LESS_EQ(sum, 1);

#ifdef PR_TEST_VERBOSE
	std::cout << std::endl;
#endif
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(BRDF);
PRT_END_MAIN