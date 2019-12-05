#pragma once

#include "BoundingBox.h"

#include <utility>

namespace PR {
/* Unbounded quadric
* Ax^2 + By^2 + Cz^2 +2Dxy + 2Exz + 2Fyz + 2Gx + 2Hy + 2Iz + J = 0
*/
class PR_LIB Quadric {
	PR_CLASS_NON_CONSTRUCTABLE(Quadric);

public:
	/*
	Solve t for 
		(a) t**2*(A*Dx**2 + B*Dy**2 + C*Dz**2 + D*Dx*Dy + Dx*Dz*E + Dy*Dz*F) 
		(b) + t*(2*A*Dx*Ox + 2*B*Dy*Oy + 2*C*Dz*Oz + D*Dx*Oy + D*Dy*Ox + Dx*E*Oz + Dx*G + Dy*F*Oz + Dy*H + Dz*E*Ox + Dz*F*Oy + Dz*I)
		(c) + A*Ox**2 + B*Oy**2 + C*Oz**2 + D*Ox*Oy + E*Ox*Oz + F*Oy*Oz + G*Ox + H*Oy + I*Oz + J
			= 0
		<=>
			t**2*a + t*b + c = 0
	*/
	template <typename T>
	inline static typename VectorTemplate<T>::bool_t intersect(
		const std::array<float, 10>& parameters,
		const RayPackageBase<T>& in,
		T& t)
	{
		constexpr T INT_EPS = T(1e-6f);

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

		T a = A * in.Direction[0] * in.Direction[0] + B * in.Direction[1] * in.Direction[1] + C * in.Direction[2] * in.Direction[2]
				  + D * in.Direction[0] * in.Direction[1] + E * in.Direction[0] * in.Direction[2] + F * in.Direction[1] * in.Direction[2];
		T b = 2 * A * in.Origin[0] * in.Direction[0] + 2 * B * in.Origin[1] * in.Direction[1] + 2 * C * in.Origin[2] * in.Direction[2]
				  + D * (in.Origin[0] * in.Direction[1] + in.Origin[1] * in.Direction[0]) + E * (in.Origin[0] * in.Direction[2] + in.Origin[2] * in.Direction[0]) + F * (in.Origin[1] * in.Direction[2] + in.Origin[2] * in.Direction[1])
				  + G * in.Direction[0] + H * in.Direction[1] + I * in.Direction[2];
		T c = A * in.Origin[0] * in.Origin[0] + B * in.Origin[1] * in.Origin[1] + C * in.Origin[2] * in.Origin[2]
				  + D * in.Origin[0] * in.Origin[1] + E * in.Origin[0] * in.Origin[2] + F * in.Origin[1] * in.Origin[2]
				  + G * in.Origin[0] + H * in.Origin[1] + I * in.Origin[2] + J;

		auto linear = a == 0;
		T lin		  = -c / b;
		T discrim	 = b * b - 4 * a * c;
		auto invalid  = discrim < 0;
		discrim		  = sqrt(discrim);
		T qu1		  = (-b - discrim) / (2 * a);
		T qu2		  = (-b + discrim) / (2 * a);
		T qu		  = blend(qu2, qu1, t < INT_EPS);

		t = blend(T(std::numeric_limits<float>::infinity()), qu, invalid);
		t = blend(lin, t, linear);

		return (t > INT_EPS);
	}

	template <typename T>
	inline static Vector3t<T> normal(
		const std::array<float, 10>& parameters,
		const Vector3t<T>& xyz) {
		const float C = parameters[2];
		const float E = parameters[4];
		const float F = parameters[5];
		const float I = parameters[8];

	}
	};
} // namespace PR
