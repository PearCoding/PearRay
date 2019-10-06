#include "Test.h"
#include "material/BRDF.h"
#include "math/Reflection.h"

using namespace PR;

#ifdef PR_DEBUG
#ifndef PR_TEST_VERBOSE
#define PR_TEST_VERBOSE
#endif
#endif

PR_BEGIN_TESTCASE(BRDF)
PR_TEST("Reflection")
{
	auto N = Vector3f(0, 1, 0);
	auto V = Vector3f(1, -1, 0).normalized();

	auto R = Reflection::reflect(V.dot(N), N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(V(0), -V(1), V(2)));
}

PR_TEST("Refraction")
{
	auto N = Vector3f(0, 1, 0);
	auto V = Vector3f(1, -1, 0).normalized();

	auto R = Reflection::refract(0.9, V.dot(N), N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0.636396, -0.771362, 0));
}

PR_TEST("Is Inside")
{
	auto N = Vector3f(0, 1, 0);

	auto V1 = Vector3f(-1, 1, 0).normalized();
	PR_CHECK_TRUE(Reflection::is_inside(V1.dot(N)));

	auto V2 = Vector3f(-1, -1, 0).normalized();
	PR_CHECK_FALSE(Reflection::is_inside(V2.dot(N)));
}

PR_TEST("Face Forward")
{
	auto N = Vector3f(0, 1, 0);

	auto V1 = Vector3f(-1, 1, 0).normalized();
	PR_CHECK_NEARLY_EQ(Reflection::faceforward(V1.dot(N), N), Vector3f(0, -1, 0));

	auto V2 = Vector3f(-1, -1, 0).normalized();
	PR_CHECK_NEARLY_EQ(Reflection::faceforward(V2.dot(N), N), N);
}

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
		float d = BRDF::ndf_blinn(i * SAMPLE_STEP, alpha);
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
		float d = BRDF::ndf_blinn(i * SAMPLE_STEP, alpha);
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
		float d = BRDF::ndf_blinn(i * SAMPLE_STEP, alpha);
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
		float d = BRDF::ndf_beckmann(i * SAMPLE_STEP, alpha);
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
		float d = BRDF::ndf_beckmann(i * SAMPLE_STEP, alpha);
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
		float d = BRDF::ndf_beckmann(i * SAMPLE_STEP, alpha);
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

PR_TEST("GGX ISO NDF")
{
	constexpr uint32 MAX_SAMPLES = 16;
	constexpr float SAMPLE_STEP  = 1.0f / MAX_SAMPLES;

#ifdef PR_TEST_VERBOSE
	std::cout << "\nAlpha 0.5\n";
#endif
	float alpha = 0.5f;
	float sum   = 0;
	for (uint32 i = 1; i < MAX_SAMPLES - 1; ++i) {
		float d = BRDF::ndf_ggx_iso(i * SAMPLE_STEP, alpha);
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
	for (uint32 i = 1; i < MAX_SAMPLES - 1; ++i) {
		float d = BRDF::ndf_ggx_iso(i * SAMPLE_STEP, alpha);
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
		float d = BRDF::ndf_ggx_iso(i * SAMPLE_STEP, alpha);
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