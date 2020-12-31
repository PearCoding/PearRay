#pragma once

#include "PR_Config.h"

#include <Eigen/Eigenvalues>
#include <array>
#include <utility>

namespace PR {
/* Unbounded quadric
* Ax^2 + By^2 + Cz^2 +Dxy + Exz + Fyz + Gx + Hy + Iz + J = 0
*/
class PR_LIB_CORE Quadric {
	PR_CLASS_NON_CONSTRUCTABLE(Quadric);

public:
	typedef std::array<float, 10> ParameterArray;

	/*
	Solve t for
		(a) t**2*(A*Dx**2 + B*Dy**2 + C*Dz**2 + D*Dx*Dy + Dx*Dz*E + Dy*Dz*F)
		(b) + t*(2*A*Dx*Ox + 2*B*Dy*Oy + 2*C*Dz*Oz + D*Dx*Oy + D*Dy*Ox + Dx*E*Oz + Dx*G + Dy*F*Oz + Dy*H + Dz*E*Ox + Dz*F*Oy + Dz*I)
		(c) + A*Ox**2 + B*Oy**2 + C*Oz**2 + D*Ox*Oy + E*Ox*Oz + F*Oy*Oz + G*Ox + H*Oy + I*Oz + J
			= 0
		<=>
			t**2*a + t*b + c = 0
	*/
	inline static bool intersect(
		const ParameterArray& parameters,
		const Vector3f& origin,
		const Vector3f& direction,
		float& t)
	{
		constexpr float INT_EPS = 1e-6f;

		const float A = parameters[0];
		const float B = parameters[1];
		const float C = parameters[2];
		const float D = parameters[3];
		const float E = parameters[4];
		const float F = parameters[5];
		const float G = parameters[6];
		const float H = parameters[7];
		const float I = parameters[8];
		const float J = parameters[9];

		float a = A * direction[0] * direction[0] + B * direction[1] * direction[1] + C * direction[2] * direction[2]
				  + D * direction[0] * direction[1] + E * direction[0] * direction[2] + F * direction[1] * direction[2];
		float b = 2 * A * origin[0] * direction[0] + 2 * B * origin[1] * direction[1] + 2 * C * origin[2] * direction[2]
				  + D * (origin[0] * direction[1] + origin[1] * direction[0]) + E * (origin[0] * direction[2] + origin[2] * direction[0]) + F * (origin[1] * direction[2] + origin[2] * direction[1])
				  + G * direction[0] + H * direction[1] + I * direction[2];
		float c = A * origin[0] * origin[0] + B * origin[1] * origin[1] + C * origin[2] * origin[2]
				  + D * origin[0] * origin[1] + E * origin[0] * origin[2] + F * origin[1] * origin[2]
				  + G * origin[0] + H * origin[1] + I * origin[2] + J;

		bool linear	  = abs(a) <= PR_EPSILON;
		float lin	  = -c / b;
		float discrim = b * b - 4 * a * c;
		bool invalid  = discrim < 0;
		discrim		  = sqrt(discrim);
		float qu1	  = (-b - discrim) / (2 * a);
		float qu2	  = (-b + discrim) / (2 * a);
		bool behind	  = qu1 <= INT_EPS;
		float qu	  = behind ? qu2 : qu1;

		t = linear ? lin : (invalid ? PR_INF : qu);

		return t < PR_INF && (t >= INT_EPS);
	}

	inline static float eval(
		const ParameterArray& parameters,
		const Vector3f& xyz)
	{
		const float A = parameters[0];
		const float B = parameters[1];
		const float C = parameters[2];
		const float D = parameters[3];
		const float E = parameters[4];
		const float F = parameters[5];
		const float G = parameters[6];
		const float H = parameters[7];
		const float I = parameters[8];
		const float J = parameters[9];

		return A * xyz(0) * xyz(0) + B * xyz(1) * xyz(1) + C * xyz(2) * xyz(2)
			   + D * xyz(0) * xyz(1) + E * xyz(0) * xyz(2) + F * xyz(1) * xyz(2)
			   + G * xyz(0) + H * xyz(1) + I * xyz(2) + J;
	}

	inline static Vector3f gradient(
		const ParameterArray& parameters,
		const Vector3f& xyz)
	{
		const float A = parameters[0];
		const float B = parameters[1];
		const float C = parameters[2];
		const float D = parameters[3];
		const float E = parameters[4];
		const float F = parameters[5];
		const float G = parameters[6];
		const float H = parameters[7];
		const float I = parameters[8];
		//const float J = parameters[9];

		Vector3f N;
		N(0) = 2 * A * xyz(0) + D * xyz(1) + E * xyz(2) + G;
		N(1) = D * xyz(0) + 2 * B * xyz(1) + F * xyz(2) + H;
		N(2) = E * xyz(0) + F * xyz(1) + 2 * C * xyz(2) + I;
		return N;
	}

	inline static Vector3f normal(
		const ParameterArray& parameters,
		const Vector3f& xyz)
	{
		return gradient(parameters, xyz).normalized();
	}

	inline static Eigen::Matrix4f matrix(
		const ParameterArray& parameters)
	{
		Eigen::Matrix4f mat;
		mat(0, 0) = parameters[0];
		mat(0, 1) = 0.5f * parameters[3];
		mat(0, 2) = 0.5f * parameters[4];
		mat(0, 3) = 0.5f * parameters[6];
		mat(1, 0) = 0.5f * parameters[3];
		mat(2, 0) = 0.5f * parameters[4];
		mat(3, 0) = 0.5f * parameters[6];

		mat(1, 1) = parameters[1];
		mat(1, 2) = 0.5f * parameters[5];
		mat(1, 3) = 0.5f * parameters[7];
		mat(2, 1) = 0.5f * parameters[5];
		mat(3, 1) = 0.5f * parameters[7];

		mat(2, 2) = parameters[2];
		mat(2, 3) = 0.5f * parameters[8];
		mat(3, 2) = 0.5f * parameters[8];

		mat(3, 3) = parameters[9];

		return mat;
	}

	typedef std::array<float, 4> EigenvalueArray;
	inline static EigenvalueArray eigenvalues(const ParameterArray& parameters)
	{
		Eigen::SelfAdjointEigenSolver<Eigen::Matrix4f> solver;
		solver.compute(matrix(parameters), Eigen::EigenvaluesOnly);

		if (solver.info() == Eigen::Success) {
			Vector4f val = solver.eigenvalues();
			return { val(0), val(1), val(2), val(3) };
		} else {
			return { 0, 0, 0, 0 };
		}
	}

	// http://mathworld.wolfram.com/QuadraticSurface.html
	enum class Classification {
		CoincidentPlane = 0,
		ImaginaryEllipsoid,
		RealEllipsoid,
		EllipticImaginaryCone,
		EllipticRealCone,
		EllipticImaginaryCylinder,
		EllipticRealCylinder,
		EllipticParaboloid,
		HyperbolicCylinder,
		HyperbolicParaboloid,
		HyperboloidOne,
		HyperboloidTwo,
		ImaginaryPlane,
		RealPlane,
		ParabolicCylinder,
		ParallelImaginaryPlane,
		ParallelRealPlane,
		Unknown
	};

	inline static bool isParametrizationSupported(Classification classification)
	{
		switch (classification) {
		case Classification::ImaginaryEllipsoid:
		case Classification::EllipticImaginaryCone:
		case Classification::EllipticImaginaryCylinder:
		case Classification::ImaginaryPlane:
		case Classification::ParallelImaginaryPlane:
		case Classification::CoincidentPlane: // TODO: Add support for planes!
		case Classification::RealPlane:
		case Classification::ParabolicCylinder:
		case Classification::ParallelRealPlane:
		case Classification::Unknown:
			return false;
		default:
			return true;
		}
	}

	inline static Classification classify(const ParameterArray& parameters)
	{
		Eigen::Matrix4f mat = matrix(parameters);

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix4f> solver_E;
		solver_E.compute(mat, Eigen::EigenvaluesOnly);

		Vector4f eigen_E;
		if (solver_E.info() == Eigen::Success)
			eigen_E = solver_E.eigenvalues();
		else
			return Classification::Unknown;

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver_e;
		solver_e.compute(mat.block<3, 3>(0, 0), Eigen::EigenvaluesOnly);

		Vector3f eigen_e;
		if (solver_e.info() == Eigen::Success)
			eigen_e = solver_e.eigenvalues();
		else
			return Classification::Unknown;

		// Determine some facts
		float det  = eigen_E(0) * eigen_E(1) * eigen_E(2) * eigen_E(3);
		int rank_e = (eigen_e(0) == 0 ? 0 : 1) + (eigen_e(1) == 0 ? 0 : 1) + (eigen_e(2) == 0 ? 0 : 1);
		int rank_E = (eigen_E(0) == 0 ? 0 : 1) + (eigen_E(1) == 0 ? 0 : 1) + (eigen_E(2) == 0 ? 0 : 1) + (eigen_E(3) == 0 ? 0 : 1);

		bool sameSign = true;
		int sign	  = 0;
		for (int i = 0; i < 3; ++i) {
			if (eigen_e(i) == 0)
				continue;

			int currentSign = eigen_e(i) < 0 ? -1 : 1;
			if (sign == 0)
				sign = currentSign;
			else if (sign != currentSign) {
				sameSign = false;
				break;
			}
		}

		if (rank_e <= 1) {
			if (rank_E <= 1)
				return Classification::CoincidentPlane;
			else if (rank_E == 2)
				return Classification::ParallelRealPlane;
			else if (rank_E == 3)
				return Classification::ParabolicCylinder;
		} else if (rank_e == 2) {
			if (rank_E == 2) {
				if (parameters[9] < 0)
					return Classification::RealPlane;
				else
					return Classification::ImaginaryPlane;
			} else if (rank_E == 3) {
				if (sameSign) {
					if (parameters[9] < 0)
						return Classification::EllipticRealCylinder;
					else
						return Classification::EllipticImaginaryCylinder;
				} else
					return Classification::HyperbolicCylinder;
			}
		} else if (rank_e == 3) {
			if (rank_E == 3) {
				if (sameSign)
					return Classification::EllipticImaginaryCone;
				else
					return Classification::EllipticRealCone;
			} else if (rank_E == 4) {
				if (sameSign) {
					if (det < 0)
						return Classification::RealEllipsoid;
					else
						return Classification::ImaginaryEllipsoid;
				} else {
					if (det < 0)
						return Classification::HyperboloidTwo;
					else
						return Classification::HyperboloidOne;
				}
			}
		}

		return Classification::Unknown;
	}

	inline static Vector3f fromEllipsoidParametrization(const ParameterArray& parameters, const Vector2f& uv)
	{
		return Vector3f(parameters[0] * (1 - uv(0) * uv(0) - uv(1) * uv(1)),
						2 * parameters[1] * uv(0),
						2 * parameters[2] * uv(1))
			   / (1 + uv(0) * uv(0) + uv(1) * uv(1));
	}
};
} // namespace PR
