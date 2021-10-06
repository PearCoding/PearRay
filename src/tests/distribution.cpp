#include "math/Distribution1D.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Distribution1D)

PR_TEST("Integral")
{
	float integral;
	Distribution1D dist(5);
	dist.generate([](size_t i) { return i; }, &integral); // {0,1,2,3,4}

	PR_CHECK_EQ(integral, 10.0f);
}

PR_TEST("PMF")
{
	Distribution1D dist(5);
	dist.generate([](size_t i) { return 1; });

	constexpr float PMF = 1 / 5.0f;
	const float pmf1	= dist.discretePdf(0);
	const float pmf2	= dist.discretePdf(2);
	const float pmf3	= dist.discretePdf(4);
	PR_CHECK_NEARLY_EQ(pmf1, PMF);
	PR_CHECK_NEARLY_EQ(pmf2, PMF);
	PR_CHECK_NEARLY_EQ(pmf3, PMF);
}

PR_TEST("PMF 2")
{
	Distribution1D dist(5);
	dist.generate([](size_t i) { return i + 1.0f; });

	auto PMF		 = [](int x) { return (x + 1) / 15.0f; };
	const float pmf1 = dist.discretePdf(0);
	const float pmf2 = dist.discretePdf(2);
	const float pmf3 = dist.discretePdf(4);
	PR_CHECK_NEARLY_EQ(pmf1, PMF(0));
	PR_CHECK_NEARLY_EQ(pmf2, PMF(2));
	PR_CHECK_NEARLY_EQ(pmf3, PMF(4));
}

PR_TEST("PDF")
{
	Distribution1D dist(5);
	dist.generate([](size_t i) { return 1; });

	constexpr float PDF = 1;
	const float pdf1	= dist.continuousPdf(0.25f);
	const float pdf2	= dist.continuousPdf(0.50f);
	const float pdf3	= dist.continuousPdf(0.75f);
	PR_CHECK_NEARLY_EQ(pdf1, PDF);
	PR_CHECK_NEARLY_EQ(pdf2, PDF);
	PR_CHECK_NEARLY_EQ(pdf3, PDF);
}

PR_TEST("Consistency Discrete")
{
	Distribution1D dist(5);
	dist.generate([](size_t i) { return std::pow(i / 16.0f, 2.0f); });

	float pdf;
	size_t x   = dist.sampleDiscrete(0.5f, pdf);
	float npdf = dist.discretePdf(x);
	PR_CHECK_EQ(npdf, pdf);
}

PR_TEST("Consistency Continous")
{
	Distribution1D dist(5);
	dist.generate([](size_t i) { return std::pow(i / 16.0f, 2.0f); });

	float pdf;
	float x	   = dist.sampleContinuous(0.5f, pdf);
	float npdf = dist.continuousPdf(x);
	PR_CHECK_EQ(npdf, pdf);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Distribution1D);
PRT_END_MAIN