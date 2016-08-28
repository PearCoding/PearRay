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
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Multiply(PM::pm_Transpose(matrix()), ray.direction()));

		Sphere sphere(PM::pm_Zero(), mRadius);
		PM::vec3 collisionPos;
		if (!sphere.intersects(local, collisionPos, t))
			return false;

		collisionPoint.P = PM::pm_Multiply(matrix(), PM::pm_SetW(collisionPos, 1));

		PM::vec3 norm = PM::pm_Normalize3D(PM::pm_Subtract(collisionPoint.P, position()));
		collisionPoint.Ng = norm;
		Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

		collisionPoint.UV = Projection::sphereUV(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), norm));

		collisionPoint.Material = material();

		t = PM::pm_Magnitude3D(PM::pm_Subtract(collisionPoint.P, ray.startPosition()));

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

		p.P = PM::pm_Multiply(matrix(), PM::pm_Scale(n, mRadius));
		p.UV = Projection::sphereUV(p.Ng);
		p.Material = material();

		return p;
	}
}