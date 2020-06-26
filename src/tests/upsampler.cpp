#include "DefaultSRGB.h"
#include "spectral/SpectralUpsampler.h"

#include "Test.h"

using namespace PR;

constexpr float EPS = 0.0001f;

PR_BEGIN_TESTCASE(SpectralUpsampler)
PR_TEST("RGB 0.8 0.2 0.3")
{
	constexpr float RGB[3]							= { 0.8f, 0.2f, 0.3f };
	constexpr float COEFFS[3]						= { 0.000110479f, -0.112288f, 27.692141f };
	constexpr std::array<float, 5> TEST_WAVELENGTHS = { 532, 615, 346, 720, 416 };
	constexpr std::array<float, 5> TEST_RESULTS		= { 0.193251f, 0.693976f, 0.950077f, 0.985873f, 0.549449f };

	auto upsampler = DefaultSRGB::loadSpectralUpsampler();

	float coeffs[3];
	upsampler->prepare(&RGB[0], &RGB[1], &RGB[2], &coeffs[0], &coeffs[1], &coeffs[2], 1);

	PR_CHECK_NEARLY_EQ_EPS(coeffs[0], COEFFS[0], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[1], COEFFS[1], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[2], COEFFS[2], EPS);

	float res[5];
	SpectralUpsampler::computeSingle(coeffs[0], coeffs[1], coeffs[2], TEST_WAVELENGTHS.data(), res, TEST_WAVELENGTHS.size());

	for (size_t i = 0; i < TEST_WAVELENGTHS.size(); ++i)
		PR_CHECK_NEARLY_EQ_EPS(res[i], TEST_RESULTS[i], EPS);
}
PR_TEST("RGB 0.2 0.8 0.4")
{
	constexpr float RGB[3]							= { 0.2f, 0.8f, 0.4f };
	constexpr float COEFFS[3]						= { -0.000149f, 0.156879f, -40.710041f };
	constexpr std::array<float, 5> TEST_WAVELENGTHS = { 532, 615, 346, 720, 416 };
	constexpr std::array<float, 5> TEST_RESULTS		= { 0.783971f, 0.299929f, 0.013470f, 0.010529f, 0.120486f };

	auto upsampler = DefaultSRGB::loadSpectralUpsampler();

	float coeffs[3];
	upsampler->prepare(&RGB[0], &RGB[1], &RGB[2], &coeffs[0], &coeffs[1], &coeffs[2], 1);

	PR_CHECK_NEARLY_EQ_EPS(coeffs[0], COEFFS[0], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[1], COEFFS[1], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[2], COEFFS[2], EPS);

	float res[5];
	SpectralUpsampler::computeSingle(coeffs[0], coeffs[1], coeffs[2], TEST_WAVELENGTHS.data(), res, TEST_WAVELENGTHS.size());

	for (size_t i = 0; i < TEST_WAVELENGTHS.size(); ++i)
		PR_CHECK_NEARLY_EQ_EPS(res[i], TEST_RESULTS[i], EPS);
}
PR_TEST("RGB 0.1 0.3 0.8")
{
	constexpr float RGB[3]							= { 0.1f, 0.3f, 0.8f };
	constexpr float COEFFS[3]						= { 0.000033f, -0.044228f, 13.931887f };
	constexpr std::array<float, 5> TEST_WAVELENGTHS = { 532, 615, 346, 720, 416 };
	constexpr std::array<float, 5> TEST_RESULTS		= { 0.337471f, 0.165104f, 0.965322f, 0.153193f, 0.882958f };

	auto upsampler = DefaultSRGB::loadSpectralUpsampler();

	float coeffs[3];
	upsampler->prepare(&RGB[0], &RGB[1], &RGB[2], &coeffs[0], &coeffs[1], &coeffs[2], 1);

	PR_CHECK_NEARLY_EQ_EPS(coeffs[0], COEFFS[0], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[1], COEFFS[1], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[2], COEFFS[2], EPS);

	float res[5];
	SpectralUpsampler::computeSingle(coeffs[0], coeffs[1], coeffs[2], TEST_WAVELENGTHS.data(), res, TEST_WAVELENGTHS.size());

	for (size_t i = 0; i < TEST_WAVELENGTHS.size(); ++i)
		PR_CHECK_NEARLY_EQ_EPS(res[i], TEST_RESULTS[i], EPS);
}
PR_TEST("RGB 1.0 1.0 1.0")
{
	constexpr float RGB[3]							= { 1.0f, 1.0f, 1.0f };
	constexpr float COEFFS[3]						= { 0.0f, 0.0f, 5e6f };
	constexpr std::array<float, 5> TEST_WAVELENGTHS = { 532, 615, 346, 720, 416 };
	constexpr std::array<float, 5> TEST_RESULTS		= { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	auto upsampler = DefaultSRGB::loadSpectralUpsampler();

	float coeffs[3];
	upsampler->prepare(&RGB[0], &RGB[1], &RGB[2], &coeffs[0], &coeffs[1], &coeffs[2], 1);

	PR_CHECK_NEARLY_EQ_EPS(coeffs[0], COEFFS[0], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[1], COEFFS[1], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[2], COEFFS[2], EPS);

	float res[5];
	SpectralUpsampler::computeSingle(coeffs[0], coeffs[1], coeffs[2], TEST_WAVELENGTHS.data(), res, TEST_WAVELENGTHS.size());

	for (size_t i = 0; i < TEST_WAVELENGTHS.size(); ++i)
		PR_CHECK_NEARLY_EQ_EPS(res[i], TEST_RESULTS[i], EPS);
}
PR_TEST("RGB 0.0 0.0 0.0")
{
	constexpr float RGB[3]							= { 0.0f, 0.0f, 0.0f };
	constexpr float COEFFS[3]						= { 0.0f, 0.0f, -500.0f };
	constexpr std::array<float, 5> TEST_WAVELENGTHS = { 532, 615, 346, 720, 416 };
	constexpr std::array<float, 5> TEST_RESULTS		= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	auto upsampler = DefaultSRGB::loadSpectralUpsampler();

	float coeffs[3];
	upsampler->prepare(&RGB[0], &RGB[1], &RGB[2], &coeffs[0], &coeffs[1], &coeffs[2], 1);

	PR_CHECK_NEARLY_EQ_EPS(coeffs[0], COEFFS[0], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[1], COEFFS[1], EPS);
	PR_CHECK_NEARLY_EQ_EPS(coeffs[2], COEFFS[2], EPS);

	float res[5];
	SpectralUpsampler::computeSingle(coeffs[0], coeffs[1], coeffs[2], TEST_WAVELENGTHS.data(), res, TEST_WAVELENGTHS.size());

	for (size_t i = 0; i < TEST_WAVELENGTHS.size(); ++i)
		PR_CHECK_NEARLY_EQ_EPS(res[i], TEST_RESULTS[i], EPS);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(SpectralUpsampler);
PRT_END_MAIN