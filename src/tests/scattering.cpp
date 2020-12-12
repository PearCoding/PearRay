#include "math/Scattering.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Scattering)
PR_TEST("Reflection")
{
	Vector3f V = Vector3f(1, 1, 1).normalized();
	Vector3f L1 = Scattering::reflect(V);
	Vector3f L2 = Scattering::reflect(V, Vector3f(0,0,1));

	PR_CHECK_NEARLY_EQ(L1, L2);
}
PR_TEST("Refraction")
{
	Vector3f V = Vector3f(1, 1, 1).normalized();
	Vector3f L1 = Scattering::refract(0.85f, V);
	Vector3f L2 = Scattering::refract(0.85f, V, Vector3f(0,0,1));

	PR_CHECK_NEARLY_EQ(L1, L2);
}
PR_TEST("Halfway Reflection")
{
	Vector3f V = Vector3f(1, 1, 1).normalized();
	Vector3f L = Vector3f(-1, 0, 1).normalized();
	Vector3f H = Scattering::halfway_reflection(V, L);

	PR_CHECK_NEARLY_EQ(H.dot(V), H.dot(L));
}
PR_TEST("Halfway Transmission")
{
	float n1  = 1;
	float n2  = 1.55;
	float eta = n1 / n2;

	Vector3f V = Vector3f(1, 1, 1).normalized();
	Vector3f L = Vector3f(-1, 0, -1).normalized();

	Vector3f H = Scattering::halfway_refractive(n1, V, n2, L);

	Vector3f L2 = Scattering::refract(eta, V, H);
	PR_CHECK_NEARLY_EQ(L2, L);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Scattering);
PRT_END_MAIN