#include "geometry/Quadric.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Quadric)

static Quadric::ParameterArray SPHERE	= { 1, 1, 1, 0, 0, 0, 0, 0, 0, -1 };
static Quadric::ParameterArray CONE		= { 1, 1, -1, 0, 0, 0, 0, 0, 0, 0 };
static Quadric::ParameterArray CYLINDER = { 1, 1, 0, 0, 0, 0, 0, 0, 0, -1 };

PR_TEST("Eval sphere")
{
	float u0 = Quadric::eval(SPHERE, Vector3f(1, 0, 0));
	float u1 = Quadric::eval(SPHERE, Vector3f(0, 2, 0));
	float u2 = Quadric::eval(SPHERE, Vector3f(0, -2, 0));

	PR_CHECK_NEARLY_EQ(u0, 0);
	PR_CHECK_NOT_NEARLY_EQ(u1, 0);
	PR_CHECK_NOT_NEARLY_EQ(u2, 0);
}

PR_TEST("Normal sphere")
{
	Vector3f n1 = Quadric::normal(SPHERE, Vector3f(1, 0, 0));
	Vector3f n2 = Quadric::normal(SPHERE, Vector3f(0, 1, 0));
	Vector3f n3 = Quadric::normal(SPHERE, Vector3f(0, 0, 1));
	PR_CHECK_NEARLY_EQ(n1, Vector3f(1, 0, 0));
	PR_CHECK_NEARLY_EQ(n2, Vector3f(0, 1, 0));
	PR_CHECK_NEARLY_EQ(n3, Vector3f(0, 0, 1));
}

PR_TEST("Intersection sphere")
{
	float t;
	bool b = Quadric::intersect(SPHERE, Vector3f(0, 0, -2), Vector3f(0, 0, 1), t);
	PR_CHECK_TRUE(b);
	PR_CHECK_NEARLY_EQ(t, 1);

	b = Quadric::intersect(SPHERE, Vector3f(0, 0, -2), Vector3f(0, 1, 0), t);
	PR_CHECK_FALSE(b);
}

PR_TEST("Eigenvalues")
{
	Quadric::EigenvalueArray values;
	values = Quadric::eigenvalues(SPHERE);
	PR_CHECK_NEARLY_EQ(values[0], -1);
	PR_CHECK_NEARLY_EQ(values[1], 1);
	PR_CHECK_NEARLY_EQ(values[2], 1);
	PR_CHECK_NEARLY_EQ(values[3], 1);

	values = Quadric::eigenvalues(CONE);
	PR_CHECK_NEARLY_EQ(values[0], -1);
	PR_CHECK_NEARLY_EQ(values[1], 0);
	PR_CHECK_NEARLY_EQ(values[2], 1);
	PR_CHECK_NEARLY_EQ(values[3], 1);

	values = Quadric::eigenvalues(CYLINDER);
	PR_CHECK_NEARLY_EQ(values[0], -1);
	PR_CHECK_NEARLY_EQ(values[1], 0);
	PR_CHECK_NEARLY_EQ(values[2], 1);
	PR_CHECK_NEARLY_EQ(values[3], 1);
}

PR_TEST("Classify")
{
	PR_CHECK_EQ(Quadric::classify(SPHERE), Quadric::C_RealEllipsoid);
	PR_CHECK_EQ(Quadric::classify(CONE), Quadric::C_EllipticRealCone);
	PR_CHECK_EQ(Quadric::classify(CYLINDER), Quadric::C_EllipticRealCylinder);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Quadric);
PRT_END_MAIN