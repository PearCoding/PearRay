#include "SphereEntity.h"
#include "ray/Ray.h"
#include "shader/FaceSample.h"
#include "geometry/Sphere.h"

#include "math/Projection.h"
#include "sampler/Sampler.h"
#include "material/Material.h"

#include "performance/Performance.h"

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

	constexpr float P = 1.6075f;
	float SphereEntity::surfaceArea(Material* m) const
	{
		PR_GUARD_PROFILE();

		if(!m || m == mMaterial)// TODO: Scale?
		{			
			const auto s = flags() & EF_LocalArea ? scale() : worldScale();
			
			const float a = PM::pm_GetX(s) * mRadius;
			const float b = PM::pm_GetY(s) * mRadius;
			const float c = PM::pm_GetZ(s) * mRadius;

			// Knud Thomsen’s Formula
			const float t = (std::pow(a*b,P) + std::pow(a*c,P) + std::pow(b*c,P)) / 3;
			return PM_4_PI_F * std::pow(t, P);
		}
		else
			return 0;
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

	bool SphereEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_GUARD_PROFILE();

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

	FaceSample SphereEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		PR_GUARD_PROFILE();
		
		FaceSample p;

		PM::vec2 s = sampler.generate2D(sample);
		PM::vec3 n = Projection::sphere(PM::pm_GetX(s), PM::pm_GetY(s), pdf);

		p.Ng = PM::pm_Normalize3D(PM::pm_Multiply(worldDirectionMatrix(), n));
		Projection::tangent_frame(p.Ng, p.Nx, p.Ny);

		p.P = PM::pm_Transform(worldMatrix(), PM::pm_SetW(PM::pm_Scale(n, mRadius), 1));
		p.UV = Projection::sphereUV(p.Ng);
		p.Material = material();

		return p;
	}
}