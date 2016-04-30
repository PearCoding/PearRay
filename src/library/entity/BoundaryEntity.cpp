#include "BoundaryEntity.h"
#include "Random.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"
#include "geometry/Plane.h"

#include "sampler/Sampler.h"
#include "sampler/Projection.h"

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

	void BoundaryEntity::setBoundingBox(const BoundingBox& box)
	{
		mBoundingBox = box;
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

	bool BoundaryEntity::isCollidable() const
	{
		return true;
	}

	BoundingBox BoundaryEntity::localBoundingBox() const
	{
		return mBoundingBox;
	}

	bool BoundaryEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		PM::vec3 vertex = PM::pm_Set(0,0,0,1);

		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction()));

		BoundingBox box = localBoundingBox();
		if (box.intersects(local, vertex))
		{
			bool found = false;
			BoundingBox::FaceSide side;

			PM::vec3 lv = PM::pm_Subtract(vertex, box.lowerBound());
			if (PM::pm_GetX(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetX(lv) > -std::numeric_limits<float>::epsilon())
			{
				side = BoundingBox::FS_Left;
				found = true;
			}
			else if (PM::pm_GetY(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetY(lv) > -std::numeric_limits<float>::epsilon())
			{
				side = BoundingBox::FS_Bottom;
				found = true;
			}
			else if (PM::pm_GetZ(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetZ(lv) > -std::numeric_limits<float>::epsilon())
			{
				side = BoundingBox::FS_Front;
				found = true;
			}

			if (!found)
			{
				PM::vec3 uv = PM::pm_Subtract(vertex, box.upperBound());
				if (PM::pm_GetX(uv) < std::numeric_limits<float>::epsilon() &&
					PM::pm_GetX(uv) > -std::numeric_limits<float>::epsilon())
				{
					side = BoundingBox::FS_Right;
				}
				else if (PM::pm_GetY(uv) < std::numeric_limits<float>::epsilon() &&
					PM::pm_GetY(uv) > -std::numeric_limits<float>::epsilon())
				{
					side = BoundingBox::FS_Top;
				}
				else
				{
					side = BoundingBox::FS_Back;
				}
			}

			collisionPoint.setVertex(PM::pm_SetW(PM::pm_Multiply(matrix(), vertex), 1));

			Plane plane = box.getFace(side);
			collisionPoint.setNormal(PM::pm_RotateWithQuat(rotation(), plane.normal()));

			if (ray.flags() & RF_NeedCollisionUV)
			{
				float u, v;
				plane.project(vertex, u, v);
				collisionPoint.setUV(PM::pm_Set(u, v));
			}
			return true;
		}
		return false;
	}

	void BoundaryEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		if (mMaterial)
		{
			mMaterial->apply(in, this, point, renderer);
		}
	}

	FacePoint BoundaryEntity::getRandomFacePoint(Sampler& sampler, Random& random) const
	{
		auto ret = sampler.generate(random);

		BoundingBox::FaceSide side = (BoundingBox::FaceSide)Projection::map(PM::pm_GetX(ret), 0, 5);// Get randomly a face

		Plane plane = localBoundingBox().getFace(side);

		FacePoint fp;
		fp.setVertex(PM::pm_Add(position(),
			PM::pm_Add(PM::pm_Scale(PM::pm_RotateWithQuat(rotation(), PM::pm_Multiply(scale(), plane.xAxis())), PM::pm_GetY(ret)),
				PM::pm_Scale(PM::pm_RotateWithQuat(rotation(), PM::pm_Multiply(scale(), plane.yAxis())), PM::pm_GetZ(ret)))));
		fp.setNormal(PM::pm_RotateWithQuat(rotation(), plane.normal()));
		fp.setUV(PM::pm_Set(PM::pm_GetY(ret), PM::pm_GetZ(ret)));
		return fp;
	}
}