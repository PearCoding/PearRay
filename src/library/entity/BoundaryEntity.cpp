#include "BoundaryEntity.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"

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

			PM::vec3 lv = PM::pm_Subtract(vertex, box.lowerBound());
			if (PM::pm_GetX(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetX(lv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(-1, 0, 0, 1));
				found = true;
			}
			else if (PM::pm_GetY(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetY(lv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(0, -1, 0, 1));
				found = true;
			}
			else if (PM::pm_GetZ(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetZ(lv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(0, 0, -1, 1));
				found = true;
			}

			if (!found)
			{
				PM::vec3 uv = PM::pm_Subtract(vertex, box.upperBound());
				if (PM::pm_GetX(uv) < std::numeric_limits<float>::epsilon() &&
					PM::pm_GetX(uv) > -std::numeric_limits<float>::epsilon())
				{
					collisionPoint.setNormal(PM::pm_Set(1, 0, 0, 1));
				}
				else if (PM::pm_GetY(uv) < std::numeric_limits<float>::epsilon() &&
					PM::pm_GetY(uv) > -std::numeric_limits<float>::epsilon())
				{
					collisionPoint.setNormal(PM::pm_Set(0, 1, 0, 1));
				}
				else
				{
					collisionPoint.setNormal(PM::pm_Set(0, 0, 1, 1));
				}
			}

			collisionPoint.setVertex(PM::pm_Multiply(matrix(), vertex));
			collisionPoint.setNormal(PM::pm_RotateWithQuat(rotation(), collisionPoint.normal()));
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

	// TODO
	FacePoint BoundaryEntity::getRandomFacePoint(Random& random) const
	{
		return FacePoint();
	}
}