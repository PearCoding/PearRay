#include "SphereEntity.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"
#include "geometry/Sphere.h"
#include "geometry/RandomRotationSphere.h"

namespace PR
{
	SphereEntity::SphereEntity(const std::string& name, float r, Entity* parent) :
		RenderEntity(name, parent), mRadius(r), mMaterial(nullptr)
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

	bool SphereEntity::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
	}

	void SphereEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* SphereEntity::material() const
	{
		return mMaterial;
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
		Sphere sphere(position(), mRadius);
		PM::vec3 collisionPos;
		if (!sphere.intersects(ray, collisionPos))
		{
			return false;
		}

		collisionPoint.setVertex(collisionPos);

		if (ray.flags() & RF_NeedCollisionNormal || ray.flags() & RF_NeedCollisionUV)
		{
			PM::vec3 norm = PM::pm_Normalize3D(PM::pm_Subtract(collisionPoint.vertex(), position()));
			collisionPoint.setNormal(norm);

			if (ray.flags() & RF_NeedCollisionUV)
			{
				PM::vec3 rotNorm = PM::pm_RotateWithQuat(rotation(), norm);
				float u = std::acos(PM::pm_GetZ(rotNorm));
				float v = std::atan2(PM::pm_GetY(rotNorm), PM::pm_GetX(rotNorm));
				collisionPoint.setUV(PM::pm_Set(u, v));
			}
		}

		return true;
	}

	void SphereEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		if (mMaterial)
		{
			mMaterial->apply(in, this, point, renderer);
		}
	}

	FacePoint SphereEntity::getRandomFacePoint(Random& random) const
	{
		FacePoint p;

		// Not really uniform...
		PM::vec3 n = RandomRotationSphere::createFast(-1, 1, -1, 1, -1, 1, random);
		p.setNormal(PM::pm_RotateWithQuat(rotation(), n));
		p.setVertex(PM::pm_Multiply(matrix(), n));
		float u = std::acos(PM::pm_GetZ(p.normal()));
		float v = std::atan2(PM::pm_GetY(p.normal()), PM::pm_GetX(p.normal()));
		p.setUV(PM::pm_Set(u, v));

		return p;
	}
}