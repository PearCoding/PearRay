#include "spectral/SpectralUpsampler.h"
#include "DefaultSRGB.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(SpectralUpsampler)
PR_TEST("eval")
{
	auto upsampler = DefaultSRGB::loadSpectralUpsampler();
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(SpectralUpsampler);
PRT_END_MAIN