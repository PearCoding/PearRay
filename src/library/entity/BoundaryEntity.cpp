#include "BoundaryEntity.h"
#include "Random.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
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

	BoundingBox BoundaryEntity::localBoundingBox() const
	{
		return mBoundingBox;
	}

	bool BoundaryEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t)
	{
		PM::vec3 vertex = PM::pm_Set(0,0,0,1);

		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction()));

		BoundingBox box = localBoundingBox();
		BoundingBox::FaceSide side;
		if (box.intersects(local, vertex, t, side))
		{
			collisionPoint.setVertex(PM::pm_SetW(PM::pm_Multiply(matrix(), vertex), 1));

			Plane plane = box.getFace(side);
			collisionPoint.setNormal(PM::pm_Normalize3D(PM::pm_RotateWithQuat(rotation(), plane.normal())));// Have to normalize again?

			if (ray.flags() & RF_NeedCollisionUV)
			{
				float u, v;
				plane.project(vertex, u, v);
				collisionPoint.setUV(PM::pm_Set(u, v));
			}

			collisionPoint.setMaterial(material());
			return true;
		}
		return false;
	}

	FacePoint BoundaryEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		auto ret = sampler.generate3D(sample);

		BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(PM::pm_GetX(ret), 0, 5);// Get randomly a face

		Plane plane = localBoundingBox().getFace(side);

		FacePoint fp;
		fp.setVertex(PM::pm_Add(position(),
			PM::pm_Add(
				PM::pm_RotateWithQuat(rotation(), PM::pm_Scale(PM::pm_Scale(plane.xAxis(), scale()), PM::pm_GetY(ret))),
				PM::pm_RotateWithQuat(rotation(), PM::pm_Scale(PM::pm_Scale(plane.yAxis(), scale()), PM::pm_GetZ(ret)))
			)));
		fp.setNormal(PM::pm_RotateWithQuat(rotation(), plane.normal()));
		fp.setUV(PM::pm_Set(PM::pm_GetY(ret), PM::pm_GetZ(ret)));
		fp.setMaterial(material());

		return fp;
	}
}