#pragma once

#include "PearMath.h"

namespace PR
{
	class PR_LIB Reflection
	{
		PR_CLASS_NON_COPYABLE(Reflection);
	public:
		/**
		* Reflects the viewing vector through the surface normal.
		* L = V - 2(N*V)N
		*
		* @param NdotV Absolute angle between N and V
		* @param N Normal of the surface point.
		* @param V Unit vector pointing TO the surface point.
		* @return Unit vector pointing FROM the surface point outwards.
		*/
		inline static PM::vec3 reflect(float NdotV, const PM::vec3& N, const PM::vec3& V)
		{
			return PM::pm_Normalize3D(PM::pm_Add(V, PM::pm_Scale(N, 2 * NdotV)));
		}

		/**
		* Refracts the ray based on the eta parameter (eta = n1/n2)
		*
		* @param eta Index ratio (n1/n2) between the two mediums.
		* @param NdotV Absolute angle between N and V
		* @param N Normal of the surface point.
		* @param V Unit vector pointing TO the surface point.
		* @return Unit vector pointing FROM the surface point outwards.
		*/
		inline static PM::vec3 refract(float eta, float NdotV, const PM::vec3& N, const PM::vec3& V)
		{
			const float k = 1 - eta*eta*(1 - NdotV*NdotV);

			if (k < 0.0f)//TOTAL REFLECTION
				return reflect(NdotV, N, V);

			const float t = eta * NdotV + std::sqrt(k);
			return PM::pm_Normalize3D(PM::pm_Subtract(PM::pm_Scale(V, eta), PM::pm_Scale(N, t)));
		}

		/**
		* Refracts the ray based on the eta parameter (eta = n1/n2) and stops when total reflection.
		*
		* @param eta Index ratio (n1/n2) between the two mediums.
		* @param NdotV Absolute angle between N and V
		* @param N Normal of the surface point.
		* @param V Unit vector pointing TO the surface point.
		* @return Unit vector pointing FROM the surface point outwards.
		*/
		inline static PM::vec3 refract(float eta, float NdotV, const PM::vec3& N, const PM::vec3& V, bool& total)
		{
			const float k = 1 - eta*eta*(1 - NdotV*NdotV);

			total = k < 0.0f;
			if (total)//TOTAL REFLECTION
				return PM::pm_Zero();

			const float t = eta * NdotV + std::sqrt(k);
			return PM::pm_Normalize3D(PM::pm_Subtract(PM::pm_Scale(V, eta), PM::pm_Scale(N, t)));
		}

		/**
		* @param NdotV dot product between normal and incident view vector
		* @param N Normal of the surface point.
		*/
		inline static PM::vec3 faceforward(float NdotV, const PM::vec3& N)
		{
			return (NdotV < 0.0f) ? N : PM::pm_Negate(N);
		}

		/**
		* Returns the halfway vector between V and L.
		* @param V Unit vector pointing TO the surface point.
		* @param L Unit vector pointing FROM the surface point outwards.
		* @return Unit vector pointing FROM the surface point outwards. (Between L and V)
		 */
		inline static PM::vec3 halfway(const PM::vec3& V, const PM::vec3& L)
		{
			return PM::pm_Normalize3D(PM::pm_Subtract(L, V));
		}
	};
}