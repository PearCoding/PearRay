#include "BoundaryEntity.h"
#include "Random.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"
#include "geometry/Plane.h"

#include "sampler/Sampler.h"
#include "math/Projection.h"
#include "material/Material.h"

namespace PR
{
	BoundaryEntity::BoundaryEntity(const std::string& name, const BoundingBox& box, Entity* parent) :
		RenderEntity(name, parent), mBoundingBox(box), mMaterial(nullptr)
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

	bool BoundaryEntity::checkCollision(const Ray& ray, SamplePoint& collisionPoint) const
	{
		PM::vec3 vertex = PM::pm_Set(0,0,0,1);

		Ray local = ray;
		local.setStartPosition(PM::pm_Transform(worldInvMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Multiply(worldInvDirectionMatrix(), ray.direction())));

		BoundingBox box = localBoundingBox();
		float t;
		BoundingBox::FaceSide side;
		if (box.intersects(local, vertex, t, side))
		{
			collisionPoint.P = PM::pm_Transform(worldMatrix(), vertex);

			Plane plane = box.getFace(side);
			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Multiply(worldDirectionMatrix(), plane.normal()));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			float u, v;
			plane.project(vertex, u, v);
			collisionPoint.UV = PM::pm_Set(u, v);
			collisionPoint.Material = material();
			return true;
		}
		return false;
	}

	SamplePoint BoundaryEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		auto ret = sampler.generate3D(sample);

		BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(PM::pm_GetX(ret), 0, 5);// Get randomly a face

		Plane plane = localBoundingBox().getFace(side);

		PM::vec xaxis = PM::pm_Multiply(worldDirectionMatrix(), plane.xAxis());
		PM::vec yaxis = PM::pm_Multiply(worldDirectionMatrix(), plane.yAxis());

		SamplePoint fp;
		fp.P = PM::pm_Add(worldPosition(),
			PM::pm_Add(PM::pm_Scale(xaxis, PM::pm_GetX(ret)),
				PM::pm_Scale(yaxis, PM::pm_GetY(ret))));
		fp.Ng = PM::pm_Normalize3D(PM::pm_Multiply(worldDirectionMatrix(), plane.normal()));
		fp.N = fp.Ng;
		Projection::tangent_frame(fp.Ng, fp.Nx, fp.Ny);
		fp.UV = PM::pm_Set(PM::pm_GetY(ret), PM::pm_GetZ(ret));
		fp.Material = material();

		return fp;
	}
}