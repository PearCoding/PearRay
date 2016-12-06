#include "BoundaryEntity.h"
#include "Random.h"
#include "ray/Ray.h"
#include "shader/FaceSample.h"
#include "geometry/Plane.h"

#include "sampler/Sampler.h"
#include "math/Projection.h"
#include "material/Material.h"

#include "performance/Performance.h"

namespace PR
{
	BoundaryEntity::BoundaryEntity(uint32 id, const std::string& name, const BoundingBox& box) :
		RenderEntity(id, name), mBoundingBox(box), mMaterial(nullptr)
	{
	}

	BoundaryEntity::~BoundaryEntity()
	{
	}

	std::string BoundaryEntity::type() const
	{
		return "boundary";
	}

	bool BoundaryEntity::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
	}

	float BoundaryEntity::surfaceArea(Material* m) const
	{
		PR_GUARD_PROFILE();

		if(!m || m == mMaterial)
		{
			if(flags() & EF_LocalArea)
				return localBoundingBox().surfaceArea();
			else
				return worldBoundingBox().surfaceArea();
		}
		else
		{
			return 0;
		}
	}

	void BoundaryEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* BoundaryEntity::material() const
	{
		return mMaterial;
	}

	void BoundaryEntity::setBoundingBox(const BoundingBox& box)
	{
		mBoundingBox = box;
	}

	bool BoundaryEntity::isCollidable() const
	{
		return true;
	}

	float BoundaryEntity::collisionCost() const
	{
		return 2;
	}

	BoundingBox BoundaryEntity::localBoundingBox() const
	{
		return mBoundingBox;
	}

	bool BoundaryEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_GUARD_PROFILE();

		PM::vec3 vertex = PM::pm_Set(0,0,0,1);

		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Multiply(invDirectionMatrix(), ray.direction())));

		BoundingBox box = localBoundingBox();
		float t;
		BoundingBox::FaceSide side;
		if (box.intersects(local, vertex, t, side))
		{
			collisionPoint.P = PM::pm_Multiply(matrix(), vertex);

			Plane plane = box.getFace(side);
			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Multiply(directionMatrix(), plane.normal()));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			float u, v;
			plane.project(vertex, u, v);
			collisionPoint.UV = PM::pm_Set(u, v);
			collisionPoint.Material = material();
			return true;
		}
		return false;
	}

	FaceSample BoundaryEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		PR_GUARD_PROFILE();

		auto ret = sampler.generate3D(sample);

		BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(PM::pm_GetX(ret), 0, 5);// Get randomly a face

		Plane plane = localBoundingBox().getFace(side);

		PM::vec xaxis = PM::pm_Multiply(directionMatrix(), plane.xAxis());
		PM::vec yaxis = PM::pm_Multiply(directionMatrix(), plane.yAxis());

		FaceSample fp;
		fp.P = PM::pm_Add(position(),
			PM::pm_Add(PM::pm_Scale(xaxis, PM::pm_GetX(ret)),
				PM::pm_Scale(yaxis, PM::pm_GetY(ret))));
		fp.Ng = PM::pm_Normalize3D(PM::pm_Multiply(directionMatrix(), plane.normal()));
		Projection::tangent_frame(fp.Ng, fp.Nx, fp.Ny);
		fp.UV = PM::pm_Set(PM::pm_GetY(ret), PM::pm_GetZ(ret));
		fp.Material = material();

		pdf = 1;//??
		return fp;
	}

	// Entity
	void BoundaryEntity::setup(RenderContext* context)
	{
		RenderEntity::setup(context);

		if(mMaterial)
			mMaterial->setup(context);
	}
}