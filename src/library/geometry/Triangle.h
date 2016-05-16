#pragma once

#include "Config.h"
#include "Face.h"
#include "FacePoint.h"

#include "ray/Ray.h"

#define PR_TRIANGLE_INTERSECT_EPSILON (1e-5f)

namespace PR
{
	class PR_LIB_INLINE Triangle
	{
	public:
		inline static bool intersect(const Ray& ray, const Face& face,
			FacePoint& point)
		{
			float u, v;
			PM::vec3 pos;

			if (intersect(ray, face.V[0], face.V[1], face.V[2], u, v, pos))
			{
				PM::vec3 p;
				PM::vec2 uv;
				PM::vec3 n;

				face.interpolate(u, v, p, n, uv);
				point.setVertex(p);
				point.setUV(uv);
				point.setNormal(n);
				return true;
			}
			else
				return false;
		}

		inline static bool intersect(const Ray& ray, const Face& face,
			float& u, float& v, PM::vec3& point)
		{
			return intersect(ray, face.V[0], face.V[1], face.V[2], u, v, point);
		}

		// Better use with normals!
		inline static bool intersect(const Ray& ray, const PM::vec3& p1, const PM::vec3& p2, const PM::vec3& p3,
			float& u, float& v, PM::vec3& point)
		{
			PM::vec3 e12 = PM::pm_Subtract(p2, p1);
			PM::vec3 e13 = PM::pm_Subtract(p3, p1);
			PM::vec3 q = PM::pm_Cross3D(ray.direction(), e13);
			float a = PM::pm_Dot3D(e12, q);

			if (a > -PR_TRIANGLE_INTERSECT_EPSILON && a < PR_TRIANGLE_INTERSECT_EPSILON)
			{
				return false;
			}

			float f = 1 / a;
			PM::vec3 s = PM::pm_Subtract(ray.startPosition(), p1);
			u = f*PM::pm_Dot3D(s, q);

			if (u < 0)
			{
				return false;
			}

			PM::vec3 r = PM::pm_Cross3D(s, e12);
			v = f*PM::pm_Dot3D(ray.direction(), r);

			if (v < 0 || u + v > 1)
			{
				return false;
			}
			
			float t = f*PM::pm_Dot3D(e13, r);
			point = PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t));
			return true;
		}

		inline BoundingBox static getBoundingBox(const PM::vec3& p1, const PM::vec3& p2, const PM::vec3& p3)
		{
			constexpr float VertexOffset = 0.00001f;

			BoundingBox box(p1, p2);
			box.put(p3);

			if (box.width() < PM_EPSILON)
				box.put(PM::pm_SetX(p1, PM::pm_GetX(p1) + VertexOffset));

			if (box.height() < PM_EPSILON)
				box.put(PM::pm_SetY(p1, PM::pm_GetY(p1) + VertexOffset));

			if (box.depth() < PM_EPSILON)
				box.put(PM::pm_SetZ(p1, PM::pm_GetZ(p1) + VertexOffset));

			return box;
		}
	};
}