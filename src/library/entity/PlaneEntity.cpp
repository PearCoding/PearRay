#include "PlaneEntity.h"

#include "ray/Ray.h"
#include "material/Material.h"
#include "shader/FaceSample.h"

#include "Logger.h"
#include "Random.h"
#include "sampler/Sampler.h"
#include "math/Projection.h"

#include "performance/Performance.h"

namespace PR
{
	PlaneEntity::PlaneEntity(uint32 id, const std::string& name, const Plane& plane) :
		RenderEntity(id, name), mPlane(plane), mMaterial(nullptr)
	{
	}

	PlaneEntity::~PlaneEntity()
	{
	}

	std::string PlaneEntity::type() const
	{
		return "plane";
	}

	bool PlaneEntity::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
	}

	float PlaneEntity::surfaceArea(Material* m) const
	{
		if(!m || m == mMaterial)// TODO: Scale?
			return mPlane.surfaceArea();
		else
			return 0;
	}

	void PlaneEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* PlaneEntity::material() const
	{
		return mMaterial;
	}

	void PlaneEntity::setPlane(const Plane& plane)
	{
		mPlane = plane;
	}

	Plane PlaneEntity::plane() const
	{
		return mPlane;
	}

	bool PlaneEntity::isCollidable() const
	{
		return true;
	}

	float PlaneEntity::collisionCost() const
	{
		return 2;
	}

	BoundingBox PlaneEntity::localBoundingBox() const
	{
		return mPlane.toBoundingBox();
	}

	bool PlaneEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_GUARD_PROFILE();

		PM::vec3 pos;
		float u, v;

		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Transform(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Multiply(invDirectionMatrix(), ray.direction())));

		float t;
		if (mPlane.intersects(local, pos, t, u, v))
		{
			collisionPoint.P = PM::pm_Transform(matrix(), pos);

			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Multiply(directionMatrix(), mPlane.normal()));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			collisionPoint.UV = PM::pm_Set(u, v);
			collisionPoint.Material = material();

			return true;
		}

		return false;
	}

	// World space
	FaceSample PlaneEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		auto s = sampler.generate2D(sample);

		FaceSample fp;
		fp.P = PM::pm_Add(mPlane.position(),
			PM::pm_Add(PM::pm_Scale(mPlane.xAxis(), PM::pm_GetX(s)),
				PM::pm_Scale(mPlane.yAxis(), PM::pm_GetY(s))));
		fp.P = PM::pm_SetW(PM::pm_Multiply(matrix(), fp.P), 1);

		fp.Ng = PM::pm_Normalize3D(PM::pm_Multiply(directionMatrix(), mPlane.normal()));
		Projection::tangent_frame(fp.Ng, fp.Nx, fp.Ny);

		fp.UV = PM::pm_SetZ(s, 0);
		fp.Material = material();

		pdf = 1;//?
		return fp;
	}

	void PlaneEntity::setup(RenderContext* context)
	{
		RenderEntity::setup(context);
		
		if(mMaterial)
			mMaterial->setup(context);
	}
}