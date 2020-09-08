#include "math/Tangent.h"
#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Tangent)
PR_TEST("Frame 1")
{
	auto N = Vector3f(0, 0, 1);

	Vector3f Nx, Ny;
	Tangent::unnormalized_frame(N, Nx, Ny);

	PR_CHECK_NEARLY_EQ(Nx, Vector3f(1, 0, 0));
	PR_CHECK_NEARLY_EQ(Ny, Vector3f(0, 1, 0));
}
PR_TEST("Frame 2")
{
	auto N = Vector3f(0, 1, 0);

	Vector3f Nx, Ny;
	Tangent::unnormalized_frame(N, Nx, Ny);

	PR_CHECK_NEARLY_EQ(Nx, Vector3f(1, 0, 0));
	PR_CHECK_NEARLY_EQ(Ny, Vector3f(0, 0, -1));
}
PR_TEST("Frame Perpendicular")
{
	auto N = Vector3f(0, 1, 0);

	Vector3f Nx, Ny;
	Tangent::unnormalized_frame(N, Nx, Ny);

	PR_CHECK_NEARLY_EQ(Nx.dot(N), 0);
	PR_CHECK_NEARLY_EQ(Ny.dot(N), 0);
	PR_CHECK_NEARLY_EQ(Nx.dot(Ny), 0);
}
PR_TEST("Align 1")
{
	auto N = Vector3f(0, 0, 1);
	auto V = Vector3f(0, 1, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, V);
}

PR_TEST("Align 2")
{
	auto N = Vector3f(0, 1, 0);
	auto V = Vector3f(0, 1, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, 0, -1));
}

PR_TEST("Align 3")
{
	auto N = Vector3f(0, 0.5f, 0.5f).normalized();
	auto V = Vector3f(0, 1, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, 0.5f, -0.5f).normalized());
}

PR_TEST("Align 4")
{
	auto N = Vector3f(1, 0, 0);
	auto V = Vector3f(0, 1, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, 1, 0));
}

PR_TEST("Align 5")
{
	auto N = Vector3f(0, 1, 0);
	auto V = Vector3f(0, 0, 1);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, N);
}

/*PR_TEST("Align 6")
{
	auto N = Vector3f(0, 0, -1);
	auto V = Vector3f(0, 1, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, -1, 0));
}

PR_TEST("Align 7")
{
	auto N = Vector3f(0, 0, -1);
	auto V = Vector3f(1, 0, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(1, 0, 0));
}*/

PR_TEST("Align 8")
{
	auto N = Vector3f(0, -1, 0);
	auto V = Vector3f(0, 1, 0);

	auto R = Tangent::align(N, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, 0, 1));
}

PR_TEST("Space 1")
{
	auto N  = Vector3f(0, 0, 1);
	auto Nx = Vector3f(1, 0, 0);
	auto Ny = Vector3f(0, 1, 0);
	auto V  = Vector3f(0, 0, 1);

	auto R = Tangent::toTangentSpace(N, Nx, Ny, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, 0, 1));
}

PR_TEST("Space 2")
{
	auto N  = Vector3f(0, 0, 1);
	auto Nx = Vector3f(1, 0, 0);
	auto Ny = Vector3f(0, 1, 0);
	auto V  = Vector3f(1, 0, 0);

	auto R = Tangent::toTangentSpace(N, Nx, Ny, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(1, 0, 0));
}

PR_TEST("Space 3")
{
	auto N  = Vector3f(0, 0, 1);
	auto Nx = Vector3f(1, 0, 0);
	auto Ny = Vector3f(0, 1, 0);
	auto V  = Vector3f(0, 1, 0);

	auto R = Tangent::toTangentSpace(N, Nx, Ny, V);

	PR_CHECK_NEARLY_EQ(R, Vector3f(0, 1, 0));
}

PR_TEST("Space Reversibility 1")
{
	auto N  = Vector3f(0, 0, 1);
	auto Nx = Vector3f(1, 0, 0);
	auto Ny = Vector3f(0, 1, 0);
	auto V  = Vector3f(0, 1, 0);

	auto T = Tangent::toTangentSpace(N, Nx, Ny, V);
	auto R = Tangent::fromTangentSpace(N, Nx, Ny, T);

	PR_CHECK_NEARLY_EQ(R, V);
}

PR_TEST("Space Reversibility 2")
{
	auto N  = Vector3f(1, 1, 0).normalized();
	Vector3f Nx, Ny;
	Tangent::unnormalized_frame(N, Nx, Ny);

	auto V  = Vector3f(0, 1, 0);

	auto T = Tangent::toTangentSpace(N, Nx, Ny, V);
	auto R = Tangent::fromTangentSpace(N, Nx, Ny, T);

	PR_CHECK_NEARLY_EQ(R, V);
}

PR_TEST("Space Reversibility 3")
{
	auto N  = Vector3f(0, 0, 1);
	auto Nx = Vector3f(1, 0, 0);
	auto Ny = Vector3f(0, 1, 0);
	auto V  = Vector3f(0, 1, 0);

	auto T = Tangent::fromTangentSpace(N, Nx, Ny, V);
	auto R = Tangent::toTangentSpace(N, Nx, Ny, T);

	PR_CHECK_NEARLY_EQ(R, V);
}

PR_TEST("Space Reversibility 4")
{
	auto N  = Vector3f(1, 1, 0).normalized();
	Vector3f Nx, Ny;
	Tangent::unnormalized_frame(N, Nx, Ny);

	auto V  = Vector3f(0, 1, 0);

	auto T = Tangent::fromTangentSpace(N, Nx, Ny, V);
	auto R = Tangent::toTangentSpace(N, Nx, Ny, T);

	PR_CHECK_NEARLY_EQ(R, V);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Tangent);
PRT_END_MAIN