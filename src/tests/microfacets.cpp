#include "Test.h"
#include "math/Microfacet.h"

using namespace PR;

PR_BEGIN_TESTCASE(Microfacets)
PR_TEST("GGX Iso")
{
	float m1		 = 0.05f;
	const Vector3f H = Vector3f(0, 0.2, 0.8).normalized();

	const float NdotH = H[2];
	float D			  = Microfacet::ndf_ggx(NdotH, m1);
	float pdf1		  = D * NdotH;
	float pdf2		  = Microfacet::pdf_ggx(NdotH, m1);
	PR_CHECK_EQ(pdf2, pdf1);
}
PR_TEST("GGX Aniso")
{
	float m1		 = 0.05f;
	float m2		 = 0.45f;
	const Vector3f H = Vector3f(0, 0.2, 0.8).normalized();

	float D	   = Microfacet::ndf_ggx(H, m1, m2);
	float pdf1 = D * H[2];
	float pdf2 = Microfacet::pdf_ggx(H, m1, m2);
	PR_CHECK_EQ(pdf2, pdf1);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Microfacets);
PRT_END_MAIN