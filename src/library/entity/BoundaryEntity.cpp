#include "BoundaryEntity.h"
#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"

namespace PR
{
	BoundaryEntity::BoundaryEntity(const std::string& name, const BoundingBox& box, Entity* parent) :
		GeometryEntity(name, parent), mBoundingBox(box)
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
		BoundingBox world = worldBoundingBox();
		if (world.intersects(ray, vertex))
		{
			collisionPoint.setVertex(vertex);

			PM::vec3 lv = PM::pm_Subtract(vertex, world.lowerBound());
			if (PM::pm_GetX(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetX(lv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(-1, 0, 0, 1));
				return true;
			}
			else if (PM::pm_GetY(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetY(lv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(0, -1, 0, 1));
				return true;
			}
			else if (PM::pm_GetZ(lv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetZ(lv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(0, 0, -1, 1));
				return true;
			}

			PM::vec3 uv = PM::pm_Subtract(vertex, world.upperBound());
			if (PM::pm_GetX(uv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetX(uv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(1, 0, 0, 1));
				return true;
			}
			else if (PM::pm_GetY(uv) < std::numeric_limits<float>::epsilon() &&
				PM::pm_GetY(uv) > -std::numeric_limits<float>::epsilon())
			{
				collisionPoint.setNormal(PM::pm_Set(0, 1, 0, 1));
				return true;
			}
			else
			{
				collisionPoint.setNormal(PM::pm_Set(0, 0, 1, 1));
				return true;
			}
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
}