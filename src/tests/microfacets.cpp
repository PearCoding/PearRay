#include "Test.h"
#include "math/Microfacet.h"
#include "math/MicrofacetReflection.h"
#include "math/MicrofacetTransmission.h"

using namespace PR;

PR_BEGIN_TESTCASE(Microfacets)
PR_TEST("GGX Iso")
{
	float m1		 = 0.05f;
	const Vector3f H = Vector3f(0, 0.2, 0.8).normalized();

	float D	   = Microfacet::ndf_ggx(H, m1);
	float pdf1 = D * H[2];
	float pdf2 = Microfacet::pdf_ggx(H, m1);
	PR_CHECK_NEARLY_EQ(pdf2, pdf1);
}
PR_TEST("GGX Aniso")
{
	float m1		 = 0.05f;
	float m2		 = 0.45f;
	const Vector3f H = Vector3f(0, 0.2, 0.8).normalized();

	float D	   = Microfacet::ndf_ggx(H, m1, m2);
	float pdf1 = D * H[2];
	float pdf2 = Microfacet::pdf_ggx(H, m1, m2);
	PR_CHECK_NEARLY_EQ(pdf2, pdf1);
}
PR_TEST("GGX Iso=Aniso")
{
	float m1		 = 0.05f;
	const Vector3f H = Vector3f(0, 0.2, 0.8).normalized();

	float D1 = Microfacet::ndf_ggx(H, m1);
	float D2 = Microfacet::ndf_ggx(H, m1, m1);
	PR_CHECK_NEARLY_EQ(D1, D2);
}
PR_TEST("Reflection Reciprocal Delta")
{
	const auto refl	 = MicrofacetReflection<false, false>(0, 0);
	const Vector3f A = Vector3f(0, 1, 1).normalized();
	const Vector3f B = Scattering::reflect(A);

	const float a = refl.eval(A, B);
	const float b = refl.eval(B, A);

	PR_CHECK_NEARLY_EQ(a, b);

	// The fresnel term has not to be reciprocal! :P
	/*const float a2 = refl.evalDielectric(A, B, 1.55f, 1.12f);
	const float b2 = refl.evalDielectric(B, A, 1.12f, 1.55f);

	PR_CHECK_NEARLY_EQ(a2, b2);*/

	const float a3 = refl.evalConductor(A, B, 0.051585, 3.9046f);
	const float b3 = refl.evalConductor(B, A, 0.051585, 3.9046f);

	PR_CHECK_NEARLY_EQ(a3, b3);

	const float a4 = refl.pdf(A, B);
	const float b4 = refl.pdf(B, A);

	PR_CHECK_NEARLY_EQ(a4, b4);
}
PR_TEST("Reflection Reciprocal")
{
	const auto refl	 = MicrofacetReflection<false, false>(0.245f, 0.245f);
	const Vector3f A = Vector3f(0, 1, 1).normalized();
	const Vector3f B = Scattering::reflect(A);

	const float a = refl.eval(A, B);
	const float b = refl.eval(B, A);

	PR_CHECK_NEARLY_EQ(a, b);

	// The fresnel term has not to be reciprocal! :P
	/*const float a2 = refl.evalDielectric(A, B, 1.55f, 1.12f);
	const float b2 = refl.evalDielectric(B, A, 1.12f, 1.55f);

	PR_CHECK_NEARLY_EQ(a2, b2);*/

	const float a3 = refl.evalConductor(A, B, 0.051585, 3.9046f);
	const float b3 = refl.evalConductor(B, A, 0.051585, 3.9046f);

	PR_CHECK_NEARLY_EQ(a3, b3);

	const float a4 = refl.pdf(A, B);
	const float b4 = refl.pdf(B, A);

	PR_CHECK_NEARLY_EQ(a4, b4);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Microfacets);
PRT_END_MAIN