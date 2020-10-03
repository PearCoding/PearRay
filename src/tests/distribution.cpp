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