#pragma once

#include "Face.h"
#include "BoundingBox.h"

#include "shader/FaceSample.h"
#include "ray/Ray.h"

#include <utility>

#include "performance/Performance.h"

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 0
# define PR_TRIANGLE_INTERSECT_EPSILON (1e-4f)
#elif PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
# define PR_TRIANGLE_INTERSECT_EPSILON (PM_EPSILON)
#else
# error Unknown Triangle intersection technique.
#endif


namespace PR
{
	class PR_LIB_INLINE Triangle
	{
	public:
		inline static bool intersect(const Ray& ray, const Face& face,
			FaceSample& point, float& t)
		{
			PR_GUARD_PROFILE();

			float u, v;
			PM::vec3 pos;

			if (intersect(ray, face.V[0], face.V[1], face.V[2], u, v, pos, t))
			{
				PM::vec3 p;
				PM::vec2 uv;
				PM::vec3 n;

				face.interpolate(u, v, p, n, uv);
				point.P = p;
				point.UVW = PM::pm_ExtendTo3D(uv);
				point.Ng = n;
				point.Material = nullptr;
				return true;
			}
			else
				return false;
		}

		inline static bool intersect(const Ray& ray, const Face& face,
			float& u, float& v, PM::vec3& point, float& t)
		{
			return intersect(ray, face.V[0], face.V[1], face.V[2], u, v, point, t);
		}

#define BETWEEN_EXCLUSIVE(v, lower, upper) (((v) - (lower)) < ((upper) - (lower)))
#define BETWEEN_INCLUSIVE(v, lower, upper) (((v) - (lower)) <= ((upper) - (lower)))

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 0
		// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
		inline static bool intersect(const Ray& ray, const PM::vec3& p1, const PM::vec3& p2, const PM::vec3& p3,
			float& u, float& v, PM::vec3& point, float& t)
		{
			PR_GUARD_PROFILE();

			const PM::vec3 e12 = PM::pm_Subtract(p2, p1);
			const PM::vec3 e13 = PM::pm_Subtract(p3, p1);
			const PM::vec3 q = PM::pm_Cross(ray.direction(), e13);
			float a = PM::pm_Dot(e12, q);

			if (a > -PR_TRIANGLE_INTERSECT_EPSILON && a < PR_TRIANGLE_INTERSECT_EPSILON)
			//if (BETWEEN_EXCLUSIVE(a, -PR_TRIANGLE_INTERSECT_EPSILON, PR_TRIANGLE_INTERSECT_EPSILON)) // Single branching
				return false;

			float f = 1.0f / a;
			const PM::vec3 s = PM::pm_Subtract(ray.startPosition(), p1);
			u = f*PM::pm_Dot(s, q);

			if (u < 0 || u > 1)
			//if (!BETWEEN_INCLUSIVE(u, 0, 1))
				return false;

			const PM::vec3 r = PM::pm_Cross(s, e12);
			v = f*PM::pm_Dot(ray.direction(), r);

			if (v < 0 || u + v > 1)
			//if (!BETWEEN_INCLUSIVE(v, 0, 1-u))
				return false;

			t = f*PM::pm_Dot(e13, r);

			if (t >= PR_TRIANGLE_INTERSECT_EPSILON)
			{
				point = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t));
				return true;
			}
			else
			{
				return false;
			}
		}
#elif PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		// Watertight
		// http://jcgt.org/published/0002/01/05/paper.pdf
		inline static bool intersect(const Ray& ray, const PM::vec3& p1, const PM::vec3& p2, const PM::vec3& p3,
			float& u, float& v, PM::vec3& point, float& t)
		{
			PR_GUARD_PROFILE();

			const uint32 kz = ray.maxDirectionIndex();
			uint32 kx = kz + 1;
			if (kx == 3)
				kx = 0;
			uint32 ky = kx + 1;
			if (ky == 3)
				ky = 0;

			if (PM::pm_GetIndex(ray.direction(), kz) < 0)
				std::swap(kx, ky);

			const float dX = PM::pm_GetIndex(ray.direction(), kx);
			const float dY = PM::pm_GetIndex(ray.direction(), ky);
			const float dZ = PM::pm_GetIndex(ray.direction(), kz);

			const float sx = dX / dZ;
			const float sy = dY / dZ;
			const float sz = 1.0f / dZ;

			// We use (1-u-v)*P1 + u*P2 + v*P3 convention
			PM::vec3 A = PM::pm_Subtract(p2, ray.startPosition());
			PM::vec3 B = PM::pm_Subtract(p3, ray.startPosition());
			PM::vec3 C = PM::pm_Subtract(p1, ray.startPosition());

			// Shear
			const float Ax = PM::pm_GetIndex(A, kx) - sx*PM::pm_GetIndex(A, kz);
			const float Ay = PM::pm_GetIndex(A, ky) - sy*PM::pm_GetIndex(A, kz);
			const float Bx = PM::pm_GetIndex(B, kx) - sx*PM::pm_GetIndex(B, kz);
			const float By = PM::pm_GetIndex(B, ky) - sy*PM::pm_GetIndex(B, kz);
			const float Cx = PM::pm_GetIndex(C, kx) - sx*PM::pm_GetIndex(C, kz);
			const float Cy = PM::pm_GetIndex(C, ky) - sy*PM::pm_GetIndex(C, kz);

			u = Cx * By - Cy * Bx;
			v = Ax * Cy - Ay * Cx;
			float w = Bx * Ay - By * Ax;

			// Better precision needed:
			if (u <= PM_EPSILON || v <= PM_EPSILON || w <= PM_EPSILON)
			{
				double CxBy = (double)Cx*(double)By;
				double CyBx = (double)Cy*(double)Bx;
				u = (float)(CxBy - CyBx);

				double AxCy = (double)Ax*(double)Cy;
				double AyCx = (double)Ay*(double)Cx;
				v = (float)(AxCy - AyCx);

				double BxAy = (double)Bx*(double)Ay;
				double ByAx = (double)By*(double)Ax;
				w = (float)(BxAy - ByAx);
			}

			if ((u < 0 || v < 0 || w < 0) && (u > 0 || v > 0 || w > 0))
				return false;

			const float det = u + v + w;
			if (std::abs(det) < PR_TRIANGLE_INTERSECT_EPSILON)
				return false;

			const float Az = sz*PM::pm_GetIndex(A, kz);
			const float Bz = sz*PM::pm_GetIndex(B, kz);
			const float Cz = sz*PM::pm_GetIndex(C, kz);

			t = u*Az + v*Bz + w*Cz;
			if (std::abs(t) >= PR_TRIANGLE_INTERSECT_EPSILON &&
				std::signbit(t) == std::signbit(det))
			{
				const float invDet = 1.0f / det;
				u *= invDet;
				v *= invDet;
				//w *= invDet;
				t *= invDet;

				if (t >= PR_TRIANGLE_INTERSECT_EPSILON)
				{
					point = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t));
					return true;
				}
				else
				{
					return false;
				}
			}

			return false;
		}
#endif

		inline static BoundingBox getBoundingBox(const PM::vec3& p1, const PM::vec3& p2, const PM::vec3& p3)
		{
			BoundingBox box(p1, p2);
			box.put(p3);

			return box;
		}

		inline static float surfaceArea(const PM::vec3& p1, const PM::vec3& p2, const PM::vec3& p3)
		{
			PR_GUARD_PROFILE();

			PM::vec3 v1 = PM::pm_Subtract(p2, p1);
			PM::vec3 v2 = PM::pm_Subtract(p3, p1);
			return 0.5f * PM::pm_Magnitude(PM::pm_Cross(v1,v2));
		}
	};
}
