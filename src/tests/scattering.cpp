#include "math/Scattering.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Scattering)
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

	Vector3f H = Scattering::halfway_transmission(n1, V, n2, L);

	float refA = Scattering::refraction_angle(H.dot(V), eta);
	PR_CHECK_NEARLY_EQ(refA, std::abs(H.dot(L)));// TODO: -1??

	Vector3f L2 = Scattering::refract(eta, refA, H.dot(V), V, H);
	PR_CHECK_NEARLY_EQ(L2, L);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Scattering);
PRT_END_MAIN