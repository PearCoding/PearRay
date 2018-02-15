#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"
#include "spectral/SpectrumLazyOperator.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Spectrum_Lazy)
PR_TEST("Add")
{
	auto desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec1 = Spectrum::gray(desc, 0.5f);
	Spectrum spec2 = Spectrum::gray(desc, 0.5f);

	auto l1 = Lazy::SLO_Add<decltype(spec1), decltype(spec2)>(spec1, spec2);
	auto l2 = Lazy::SLO_Add<decltype(spec1), decltype(spec2)>(spec1, spec2);
	auto l3 = l1+l2;
	for(uint32 i = 0; i < spec1.samples(); ++i) {
		PR_CHECK_EQ(l1(i), 1);
		PR_CHECK_EQ(l2(i), 1);
		PR_CHECK_EQ(l3(i), 2);
	}
}
PR_TEST("Sub")
{
	auto desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec1 = Spectrum::gray(desc, 0.5f);
	Spectrum spec2 = Spectrum::gray(desc, 0.5f);

	auto l1 = Lazy::SLO_Sub<decltype(spec1), decltype(spec2)>(spec1, spec2);
	auto l2 = Lazy::SLO_Sub<decltype(spec1), decltype(spec2)>(spec1, spec2);
	auto l3 = l1-l2;
	for(uint32 i = 0; i < spec1.samples(); ++i) {
		PR_CHECK_EQ(l1(i), 0);
		PR_CHECK_EQ(l2(i), 0);
		PR_CHECK_EQ(l3(i), 0);
	}
}
PR_TEST("Mul")
{
	auto desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec1 = Spectrum::gray(desc, 0.5f);
	Spectrum spec2 = Spectrum::gray(desc, 0.5f);

	auto l1 = Lazy::SLO_Mul<decltype(spec1), decltype(spec2)>(spec1, spec2);
	auto l2 = Lazy::SLO_Mul<decltype(spec1), decltype(spec2)>(spec2, spec1);
	auto l3 = l1*l2;
	for(uint32 i = 0; i < spec1.samples(); ++i) {
		PR_CHECK_EQ(l1(i), 0.25f);
		PR_CHECK_EQ(l2(i), 0.25f);
		PR_CHECK_EQ(l3(i), 0.0625f);
	}
}
PR_TEST("Div")
{
	auto desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec1 = Spectrum::gray(desc, 0.5f);
	Spectrum spec2 = Spectrum::gray(desc, 0.5f);

	auto l1 = Lazy::SLO_Div<decltype(spec1), decltype(spec2)>(spec1, spec2);
	auto l2 = Lazy::SLO_Div<decltype(spec1), decltype(spec2)>(spec2, spec1);
	auto l3 = l1/l2;
	for(uint32 i = 0; i < spec1.samples(); ++i) {
		PR_CHECK_EQ(l1(i), 1);
		PR_CHECK_EQ(l2(i), 1);
		PR_CHECK_EQ(l3(i), 1);
	}
}
PR_TEST("Scale")
{
	auto desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec1 = Spectrum::gray(desc, 0.5f);

	auto l1 = Lazy::SLO_Scale<decltype(spec1), float>(spec1, 2);
	auto l2 = Lazy::SLO_Scale<decltype(spec1), float>(spec1, 4);
	auto l3 = l1*l2;
	for(uint32 i = 0; i < spec1.samples(); ++i) {
		PR_CHECK_EQ(l1(i), 1);
		PR_CHECK_EQ(l2(i), 2);
		PR_CHECK_EQ(l3(i), 2);
	}
}
PR_TEST("InvScale")
{
	auto desc = SpectrumDescriptor::createStandardSpectral();
	Spectrum spec1 = Spectrum::gray(desc, 0.5f);

	auto l1 = Lazy::SLO_InvScale<decltype(spec1), float>(spec1, 1);
	auto l2 = Lazy::SLO_InvScale<decltype(spec1), float>(spec1, 2);
	auto l3 = l1/l2;
	for(uint32 i = 0; i < spec1.samples(); ++i) {
		PR_CHECK_EQ(l1(i), 2);
		PR_CHECK_EQ(l2(i), 4);
		PR_CHECK_EQ(l3(i), 0.5);
	}
}
PR_END_TESTCASE()

PRT_BEGIN_MAIN
PRT_TESTCASE(Spectrum_Lazy);
PRT_END_MAIN