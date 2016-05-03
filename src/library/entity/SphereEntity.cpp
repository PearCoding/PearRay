#include "SphereEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/Sphere.h"

#include "sampler/Projection.h"
#include "sampler/Sampler.h"

namespace PR
{
	SphereEntity::SphereEntity(const std::string& name, float r, Entity* parent) :
		RenderEntity(name, parent), mRadius(r)
	{
	}

	SphereEntity::~SphereEntity()
	{
	}

	std::string SphereEntity::type() const
	{
		return "sphere";
	}

	void SphereEntity::setRadius(float f)
	{
		mRadius = f;
	}

	float SphereEntity::radius() const
	{
		return mRadius;
	}

	bool SphereEntity::isCollidable() const
	{
		return true;
	}

	BoundingBox SphereEntity::localBoundingBox() const
	{
		return BoundingBox(PM::pm_Set(mRadius, mRadius, mRadius, 1),
			PM::pm_Set(-mRadius, -mRadius, -mRadius, 1));
	}

	bool SphereEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		Sphere sphere(position(), scale() * mRadius);
		PM::vec3 collisionPos;
		if (!sphere.intersects(ray, collisionPos))
			return false;

		collisionPoint.setVertex(PM::pm_SetW(collisionPos, 1));

		if (ray.flags() & RF_NeedCollisionNormal || ray.flags() & RF_NeedCollisionUV)
		{
			PM::vec3 norm = PM::pm_Normalize3D(PM::pm_Subtract(collisionPoint.vertex(), position()));
			collisionPoint.setNormal(norm);

			if (ray.flags() & RF_NeedCollisionUV)
			{
				PM::vec3 rotNorm = PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), norm);
				float u = (std::acos(PM::pm_GetZ(rotNorm)) * PM_INV_PI_F * 0.5f + 1) * 0.5f;
				float v = (std::atan2(PM::pm_GetY(rotNorm), PM::pm_GetX(rotNorm)) * PM_INV_PI_F + 1) * 0.5f;
				collisionPoint.setUV(PM::pm_Set(u, v));
			}
		}

		return true;
	}

	FacePoint SphereEntity::getRandomFacePoint(Sampler& sampler, Random& random) const
	{
		//auto sample = sampler.generate(random);

		FacePoint p;
		// Not really uniform...
		//PM::vec3 n = Projection::sphereFast(PM::pm_GetX(sample), PM::pm_GetY(sample), PM::pm_GetZ(sample));
		//PM::vec3 n = Projection::sphere(PM::pm_GetX(sample), PM::pm_GetY(sample));
		PM::vec3 n = Projection::sphereReject(random);
		//p.setNormal(n);
		p.setNormal(PM::pm_RotateWithQuat(rotation(), n));
		p.setVertex(PM::pm_Add(position(), PM::pm_Scale(n, scale() * mRadius)));
		float u = (std::acos(PM::pm_GetZ(p.normal())) * PM_INV_PI_F * 0.5f + 1) * 0.5f;
		float v = (std::atan2(PM::pm_GetY(p.normal()), PM::pm_GetX(p.normal())) * PM_INV_PI_F + 1) * 0.5f;
		p.setUV(PM::pm_Set(u, v));

		return p;
	}
}