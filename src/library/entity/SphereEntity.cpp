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

	bool SphereEntity::checkCollision(const Ray& ray, SamplePoint& collisionPoint, float& t)
	{
		Sphere sphere(position(), scale() * mRadius);
		PM::vec3 collisionPos;
		if (!sphere.intersects(ray, collisionPos, t))
			return false;

		collisionPoint.P = PM::pm_SetW(collisionPos, 1);

		PM::vec3 norm = PM::pm_Normalize3D(PM::pm_Subtract(collisionPoint.P, position()));
		collisionPoint.Ng = norm;
		Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

		PM::vec3 rotNorm = PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), norm);
		collisionPoint.UV = Projection::sphereUV(rotNorm);

		collisionPoint.Material = material();
		return true;
	}

	SamplePoint SphereEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		SamplePoint p;

		PM::vec2 s = sampler.generate2D(sample);
		PM::vec3 n = Projection::sphere(PM::pm_GetX(s), PM::pm_GetY(s));

		p.Ng = PM::pm_RotateWithQuat(rotation(), n);
		p.N = p.Ng;
		Projection::tangent_frame(p.Ng, p.Nx, p.Ny);

		p.P = PM::pm_Add(position(), PM::pm_Scale(n, scale() * mRadius));
		p.UV = Projection::sphereUV(p.Ng);
		p.Material = material();

		return p;
	}
}