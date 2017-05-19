#pragma once

#include <Eigen/Dense>

namespace PR
{
	class PR_LIB Reflection
	{
		PR_CLASS_NON_COPYABLE(Reflection);
	public:
		/**
		* @brief Reflects the viewing vector through the surface normal.
		* L = V - 2(N*V)N
		*
		* @param NdotV Angle between N and V
		* @param N Normal of the surface point.
		* @param V Unit vector pointing TO the surface point.
		* @return Unit vector pointing FROM the surface point outwards.
		*/
		inline static Eigen::Vector3f reflect(float NdotV, const Eigen::Vector3f& N, const Eigen::Vector3f& V)
		{
			return (V - N * 2 * NdotV).normalized();
		}

		/**
		* @brief Refracts the ray based on the eta parameter (eta = n1/n2)
		*
		* @param eta Index ratio (n1/n2) between the two mediums.
		* @param NdotV Angle between N and V
		* @param N Normal of the surface point.
		* @param V Unit vector pointing TO the surface point.
		* @return Unit vector pointing FROM the surface point outwards.
		*/
		inline static Eigen::Vector3f refract(float eta, float NdotV, const Eigen::Vector3f& N, const Eigen::Vector3f& V)
		{
			const float k = 1 - eta*eta*(1 - NdotV*NdotV);

			if (k < 0.0f)//TOTAL REFLECTION
				return reflect(NdotV, N, V);

			const float t = eta * NdotV + std::sqrt(k);
			return (V*eta - N*t).normalized();
		}

		/**
		* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and stops when total reflection.
		*
		* @param eta Index ratio (n1/n2) between the two mediums.
		* @param NdotV Angle between N and V
		* @param N Normal of the surface point.
		* @param V Unit vector pointing TO the surface point.
		* @return Unit vector pointing FROM the surface point outwards.
		*/
		inline static Eigen::Vector3f refract(float eta, float NdotV, const Eigen::Vector3f& N, const Eigen::Vector3f& V, bool& total)
		{
			const float k = 1 - eta*eta*(1 - NdotV*NdotV);

			total = k < 0.0f;
			if (total)//TOTAL REFLECTION
				return Eigen::Vector3f(0,0,0);

			const float t = eta * NdotV + std::sqrt(k);
			return (V*eta - N*t).normalized();
		}

		/**
		* @param NdotV dot product between normal and incident view vector
		* @param N Normal of the surface point.
		*/
		inline static Eigen::Vector3f faceforward(float NdotV, const Eigen::Vector3f& N)
		{
			return is_inside(NdotV) ? -N : N;
		}

		/**
		* @param NdotV dot product between normal and incident view vector
		*/
		inline static bool is_inside(float NdotV)
		{
			return NdotV > 0.0f;
		}

		/**
		* @brief Returns the halfway vector between V and L.
		* @param V Unit vector pointing TO the surface point.
		* @param L Unit vector pointing FROM the surface point outwards.
		* @return Unit vector pointing FROM the surface point outwards. (Between L and V)
		 */
		inline static Eigen::Vector3f halfway(const Eigen::Vector3f& V, const Eigen::Vector3f& L)
		{
			return (L-V).normalized();
		}
	};
}
