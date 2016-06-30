#include "SphereEntity.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/Sphere.h"

#include "math/Projection.h"
#include "sampler/Sampler.h"
#include "material/Material.h"

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

	float SphereEntity::collisionCost() const
	{
		return 1;
	}

	BoundingBox SphereEntity::localBoundingBox() const
	{
		return BoundingBox(PM::pm_Set(mRadius, mRadius, mRadius, 1),
			PM::pm_Set(-mRadius, -mRadius, -mRadius, 1));
	}

	bool SphereEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t)
	{
		Sphere sphere(position(), scale() * mRadius);
		PM::vec3 collisionPos;
		if (!sphere.intersects(ray, collisionPos, t))
			return false;

		collisionPoint.setVertex(PM::pm_SetW(collisionPos, 1));

		PM::vec3 norm = PM::pm_Normalize3D(PM::pm_Subtract(collisionPoint.vertex(), position()));
		collisionPoint.setNormal(norm);

		PM::vec3 rotNorm = PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), norm);
		float u = 0.5f + std::atan2(PM::pm_GetZ(rotNorm), PM::pm_GetX(rotNorm)) * PM_INV_PI_F * 0.5f;
		float v = 0.5f - std::asin(-PM::pm_GetY(rotNorm)) * PM_INV_PI_F;
		collisionPoint.setUV(PM::pm_Set(u, v));

		collisionPoint.setMaterial(material());
		return true;
	}

	FacePoint SphereEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		FacePoint p;

		PM::vec2 s = sampler.generate2D(sample);
		PM::vec3 n = Projection::sphere(PM::pm_GetX(s), PM::pm_GetY(s));

		p.setNormal(PM::pm_RotateWithQuat(rotation(), n));
		p.setVertex(PM::pm_Add(position(), PM::pm_Scale(n, scale() * mRadius)));
		float u = (std::acos(PM::pm_GetZ(p.normal())) * PM_INV_PI_F * 0.5f + 1) * 0.5f;
		float v = (std::atan2(PM::pm_GetY(p.normal()), PM::pm_GetX(p.normal())) * PM_INV_PI_F + 1) * 0.5f;
		p.setUV(PM::pm_Set(u, v));
		p.setMaterial(material());

		return p;
	}
}