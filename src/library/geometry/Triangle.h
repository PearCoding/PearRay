#pragma once

#include "BoundingBox.h"
#include "Face.h"

#include "ray/Ray.h"
#include "shader/FacePoint.h"

#include <utility>

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 0
#define PR_TRIANGLE_INTERSECT_EPSILON (1e-4f)
#elif PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
#define PR_TRIANGLE_INTERSECT_EPSILON (PR_EPSILON)
#else
#error Unknown Triangle intersection technique.
#endif

namespace PR {
class PR_LIB_INLINE Triangle {
	PR_CLASS_NON_CONSTRUCTABLE(Triangle);

public:
	inline static bool intersect(const Ray& ray, const Face& face,
								 FacePoint& point, float& t)
	{
		float u, v;
		Eigen::Vector3f pos;

		if (intersect(ray, face.V[0], face.V[1], face.V[2], u, v, pos, t)) {
			Eigen::Vector3f p;
			Eigen::Vector2f uv;
			Eigen::Vector3f n;

			face.interpolate(u, v, p, n, uv);
			point.P		   = p;
			point.UVW	  = Eigen::Vector3f(uv(0), uv(1), 0);
			point.Ng	   = n;
			point.Material = nullptr;
			return true;
		} else {
			return false;
		}
	}

	inline static bool intersect(const Ray& ray, const Face& face,
								 float& u, float& v, Eigen::Vector3f& point, float& t)
	{
		return intersect(ray, face.V[0], face.V[1], face.V[2], u, v, point, t);
	}

#define BETWEEN_EXCLUSIVE(v, lower, upper) (((v) - (lower)) < ((upper) - (lower)))
#define BETWEEN_INCLUSIVE(v, lower, upper) (((v) - (lower)) <= ((upper) - (lower)))

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 0
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	inline static bool intersect(const Ray& ray, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3,
								 float& u, float& v, Eigen::Vector3f& point, float& t)
	{
		

		const Eigen::Vector3f e12 = p2 - p1;
		const Eigen::Vector3f e13 = p3 - p1;
		const Eigen::Vector3f q   = ray.direction().cross(e13);
		float a					  = e12.dot(q);

		if (a > -PR_TRIANGLE_INTERSECT_EPSILON && a < PR_TRIANGLE_INTERSECT_EPSILON)
			//if (BETWEEN_EXCLUSIVE(a, -PR_TRIANGLE_INTERSECT_EPSILON, PR_TRIANGLE_INTERSECT_EPSILON)) // Single branching
			return false;

		float f					= 1.0f / a;
		const Eigen::Vector3f s = ray.origin() - p1;
		u						= f * s.dot(q);

		if (u < 0 || u > 1)
			//if (!BETWEEN_INCLUSIVE(u, 0, 1))
			return false;

		const Eigen::Vector3f r = s.cross(e12);
		v						= f * ray.direction().dot(r);

		if (v < 0 || u + v > 1)
			//if (!BETWEEN_INCLUSIVE(v, 0, 1-u))
			return false;

		t = f * e13.dot(r);

		if (t >= PR_TRIANGLE_INTERSECT_EPSILON) {
			point = ray.origin() + ray.direction() * t;
			return true;
		} else {
			return false;
		}
	}
#elif PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
	// Watertight
	// http://jcgt.org/published/0002/01/05/paper.pdf
	inline static bool intersect(const Ray& ray, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3,
								 float& u, float& v, Eigen::Vector3f& point, float& t)
	{
		

		const uint32 kz = ray.maxDirectionIndex();
		uint32 kx		= kz + 1;
		if (kx == 3)
			kx = 0;
		uint32 ky = kx + 1;
		if (ky == 3)
			ky = 0;

		if (ray.direction()(kz) < 0)
			std::swap(kx, ky);

		const float dX = ray.direction()(kx);
		const float dY = ray.direction()(ky);
		const float dZ = ray.direction()(kz);

		const float sx = dX / dZ;
		const float sy = dY / dZ;
		const float sz = 1.0f / dZ;

		// We use (1-u-v)*P1 + u*P2 + v*P3 convention
		Eigen::Vector3f A = p2 - ray.origin();
		Eigen::Vector3f B = p3 - ray.origin();
		Eigen::Vector3f C = p1 - ray.origin();

		// Shear
		const float Ax = A(kx) - sx * A(kz);
		const float Ay = A(ky) - sy * A(kz);
		const float Bx = B(kx) - sx * B(kz);
		const float By = B(ky) - sy * B(kz);
		const float Cx = C(kx) - sx * C(kz);
		const float Cy = C(ky) - sy * C(kz);

		u		= Cx * By - Cy * Bx;
		v		= Ax * Cy - Ay * Cx;
		float w = Bx * Ay - By * Ax;

		// Better precision needed:
		if (u <= PR_EPSILON || v <= PR_EPSILON || w <= PR_EPSILON) {
			double CxBy = static_cast<double>(Cx) * static_cast<double>(By);
			double CyBx = static_cast<double>(Cy) * static_cast<double>(Bx);
			u			= static_cast<float>(CxBy - CyBx);

			double AxCy = static_cast<double>(Ax) * static_cast<double>(Cy);
			double AyCx = static_cast<double>(Ay) * static_cast<double>(Cx);
			v			= static_cast<float>(AxCy - AyCx);

			double BxAy = static_cast<double>(Bx) * static_cast<double>(Ay);
			double ByAx = static_cast<double>(By) * static_cast<double>(Ax);
			w			= static_cast<float>(BxAy - ByAx);
		}

		if ((u < 0 || v < 0 || w < 0) && (u > 0 || v > 0 || w > 0))
			return false;

		const float det = u + v + w;
		if (std::abs(det) < PR_TRIANGLE_INTERSECT_EPSILON)
			return false;

		const float Az = sz * A(kz);
		const float Bz = sz * B(kz);
		const float Cz = sz * C(kz);

		t = u * Az + v * Bz + w * Cz;
		if (std::abs(t) >= PR_TRIANGLE_INTERSECT_EPSILON && std::signbit(t) == std::signbit(det)) {
			const float invDet = 1.0f / det;
			u *= invDet;
			v *= invDet;
			//w *= invDet;
			t *= invDet;

			if (t >= PR_TRIANGLE_INTERSECT_EPSILON) {
				point = ray.origin() + ray.direction() * t;
				return true;
			} else {
				return false;
			}
		}

		return false;
	}
#endif

	inline static BoundingBox getBoundingBox(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3)
	{
		BoundingBox box(p1, p2);
		box.combine(p3);
		box.inflate(0.0001f);

		return box;
	}

	inline static float surfaceArea(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3)
	{
		

		Eigen::Vector3f v1 = p2 - p1;
		Eigen::Vector3f v2 = p3 - p1;
		return 0.5f * v1.cross(v2).norm();
	}
};
} // namespace PR
