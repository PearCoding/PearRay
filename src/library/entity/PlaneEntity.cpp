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
		if(!m || m == mMaterial)
		{
			if((flags() & EF_LocalArea) == 0)
			{
				if(isFrozen())
					return mGlobalPlane_Cache.surfaceArea();
				else
					return std::sqrt(PM::pm_MagnitudeSqr3D(PM::pm_Multiply(directionMatrix(), mPlane.xAxis())) *
							PM::pm_MagnitudeSqr3D(PM::pm_Multiply(directionMatrix(), mPlane.yAxis())));
			}
			else
			{
				return mPlane.surfaceArea();
			}
		}
		else
		{
			return 0;
		}
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
		return mMaterial && mMaterial->canBeShaded();
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

		float t;
		if (mGlobalPlane_Cache.intersects(ray, pos, t, u, v))
		{
			collisionPoint.P = pos;

			collisionPoint.Ng = mGlobalPlane_Cache.normal();
			collisionPoint.Nx = mGlobalPlane_Cache.xAxis();
			collisionPoint.Ny = mGlobalPlane_Cache.yAxis();

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
		fp.P = PM::pm_Add(mGlobalPlane_Cache.position(),
			PM::pm_Add(PM::pm_Scale(mGlobalPlane_Cache.xAxis(), PM::pm_GetX(s)),
				PM::pm_Scale(mGlobalPlane_Cache.yAxis(), PM::pm_GetY(s))));

		fp.Ng = mGlobalPlane_Cache.normal();
		fp.Nx = mGlobalPlane_Cache.xAxis();
		fp.Ny = mGlobalPlane_Cache.yAxis();

		fp.UV = s;
		fp.Material = material();

		pdf = 1;//?
		return fp;
	}

	void PlaneEntity::onFreeze()
	{
		RenderEntity::onFreeze();

		mGlobalPlane_Cache.setPosition(PM::pm_SetW(PM::pm_Multiply(matrix(), PM::pm_SetW(mPlane.position(), 1)), 1));
		mGlobalPlane_Cache.setAxis(
			PM::pm_SetW(PM::pm_Multiply(directionMatrix(), PM::pm_SetW(mPlane.xAxis(), 0)), 0),
			PM::pm_SetW(PM::pm_Multiply(directionMatrix(), PM::pm_SetW(mPlane.yAxis(), 0)), 0));
		
		// Check up
		if(std::abs(PM::pm_MagnitudeSqr3D(mGlobalPlane_Cache.normal()) - 1) > PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has a non unit normal vector!", name().c_str());

		if(PM::pm_MagnitudeSqr3D(mGlobalPlane_Cache.xAxis()) <= PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has zero x axis!", name().c_str());

		if(PM::pm_MagnitudeSqr3D(mGlobalPlane_Cache.yAxis()) <= PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has zero y axis!", name().c_str());

		if(mGlobalPlane_Cache.surfaceArea() <= PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Plane entity %s has zero enclosed area!", name().c_str());
	}

	void PlaneEntity::setup(RenderContext* context)
	{
		RenderEntity::setup(context);
		
		if(mMaterial)
			mMaterial->setup(context);
	}
}