#include "SphereEntity.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"
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

	bool SphereEntity::checkCollision(const Ray& ray, SamplePoint& collisionPoint)
	{
		Ray local = ray;
		local.setStartPosition(PM::pm_Transform(worldInvMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Transform(worldInvDirectionMatrix(), ray.direction())));

		Sphere sphere(PM::pm_Zero(), mRadius);
		float t;
		PM::vec3 collisionPos;
		if (!sphere.intersects(local, collisionPos, t))
			return false;

		collisionPoint.P = PM::pm_Transform(worldMatrix(), collisionPos);

		collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Transform(worldDirectionMatrix(), collisionPos));
		Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

		collisionPoint.UV = Projection::sphereUV(PM::pm_RotateWithQuat(PM::pm_InverseQuat(worldRotation()), collisionPoint.Ng));

		collisionPoint.Material = material();

		return true;
	}

	SamplePoint SphereEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		SamplePoint p;

		PM::vec2 s = sampler.generate2D(sample);
		PM::vec3 n = Projection::sphere(PM::pm_GetX(s), PM::pm_GetY(s));

		p.Ng = PM::pm_Normalize3D(PM::pm_Multiply(worldDirectionMatrix(), n));
		p.N = p.Ng;
		Projection::tangent_frame(p.Ng, p.Nx, p.Ny);

		p.P = PM::pm_Transform(worldMatrix(), PM::pm_SetW(PM::pm_Scale(n, mRadius), 1));
		p.UV = Projection::sphereUV(p.Ng);
		p.Material = material();

		return p;
	}
}