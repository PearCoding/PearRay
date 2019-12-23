#include "spectral/RGBConverter.h"
#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Spectrum)
PR_TEST("Set/Get")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());

	spec.setValue(52, 1);
	PR_CHECK_EQ(spec.value(52), 1);
}
PR_TEST("Fill")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	spec.fill(1);

	for (uint32 i = 0; i < spec.samples(); ++i)
		PR_CHECK_EQ(spec(i), 1);
}
PR_TEST("Max/Min/Avg/Sum")
{
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral(), 0);

	spec(52) = 1;
	spec(42) = -1;

	PR_CHECK_EQ(spec.max(), 1);
	PR_CHECK_EQ(spec.min(), 0);// Amplitude!
	PR_CHECK_EQ(spec.avg(), 0);
	PR_CHECK_EQ(spec.sum(), 0);
	PR_CHECK_EQ(spec.sqrSum(), 2);
}

PR_END_TESTCASE()

PRT_BEGIN_MAIN
PRT_TESTCASE(Spectrum);
PRT_END_MAIN