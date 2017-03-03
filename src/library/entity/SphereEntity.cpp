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
	SphereEntity::SphereEntity(uint32 id, const std::string& name, float r) :
		RenderEntity(id, name), mRadius(r), mMaterial(nullptr)
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

		if(!m || m == mMaterial.get())
		{
			const auto s = flags() & EF_LocalArea ? PM::pm_Set(1,1,1) : scale();

			const float a = PM::pm_GetX(s) * mRadius;
			const float b = PM::pm_GetY(s) * mRadius;
			const float c = PM::pm_GetZ(s) * mRadius;

			// Knud Thomsen’s Formula
			const float t = (std::pow(a*b,P) + std::pow(a*c,P) + std::pow(b*c,P)) / 3;
			return PM_4_PI_F * std::pow(t, 1/P);
		}
		else
			return 0;
	}

	void SphereEntity::setMaterial(const std::shared_ptr<Material>& m)
	{
		mMaterial = m;
	}

	const std::shared_ptr<Material>& SphereEntity::material() const
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
		return mMaterial && mMaterial->canBeShaded() && mRadius >= PM_EPSILON;
	}

	float SphereEntity::collisionCost() const
	{
		return 1;
	}

	BoundingBox SphereEntity::localBoundingBox() const
	{
		return BoundingBox(PM::pm_Set(mRadius, mRadius, mRadius),
			PM::pm_Set(-mRadius, -mRadius, -mRadius));
	}

	bool SphereEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_GUARD_PROFILE();

		Ray local = ray;
		local.setStartPosition(PM::pm_Transform(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize(PM::pm_Transform(invDirectionMatrix(), ray.direction())));

		Sphere sphere(PM::pm_Zero3D(), mRadius);
		float t;
		PM::vec3 collisionPos;
		if (!sphere.intersects(local, collisionPos, t))
			return false;

		collisionPoint.P = PM::pm_Transform(matrix(), collisionPos);

		collisionPoint.Ng = PM::pm_Normalize(PM::pm_Transform(directionMatrix(), collisionPos));
		Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

		collisionPoint.UVW = PM::pm_ExtendTo3D(
			Projection::sphereUV(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), collisionPoint.Ng)));

		collisionPoint.Material = material().get();

		return true;
	}

	FaceSample SphereEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		PR_GUARD_PROFILE();

		FaceSample p;

		PM::vec2 s = sampler.generate2D(sample);
		PM::vec3 n = Projection::sphere_coord(PM::pm_GetX(s)*PM_2_PI_F, PM::pm_GetY(s)*PM_PI_F);
		pdf = 1;

		p.Ng = PM::pm_Normalize(PM::pm_Transform(directionMatrix(), n));
		Projection::tangent_frame(p.Ng, p.Nx, p.Ny);

		p.P = PM::pm_Transform(matrix(), PM::pm_Scale(n, mRadius));
		p.UVW = PM::pm_ExtendTo3D(
			Projection::sphereUV(p.Ng));
		p.Material = material().get();

		return p;
	}

	void SphereEntity::setup(RenderContext* context)
	{
		RenderEntity::setup(context);

		if(mMaterial)
			mMaterial->setup(context);
	}
}
