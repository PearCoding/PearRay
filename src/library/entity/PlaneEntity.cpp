#include "PlaneEntity.h"

#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"

#include "Logger.h"
#include "Random.h"

namespace PR
{
	PlaneEntity::PlaneEntity(const std::string& name, const Plane& plane, Entity* parent) :
		RenderEntity(name, parent), mPlane(plane), mMaterial(nullptr)
	{
	}

	PlaneEntity::~PlaneEntity()
	{
	}

	std::string PlaneEntity::type() const
	{
		return "plane";
	}

	void PlaneEntity::setPlane(const Plane& plane)
	{
		mPlane = plane;
	}

	Plane PlaneEntity::plane() const
	{
		return mPlane;
	}

	bool PlaneEntity::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
	}

	void PlaneEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* PlaneEntity::material() const
	{
		return mMaterial;
	}

	bool PlaneEntity::isCollidable() const
	{
		return true;
	}

	BoundingBox PlaneEntity::localBoundingBox() const
	{
		return mPlane.toLocalBoundingBox();
	}

	bool PlaneEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		PM::vec3 pos;
		float u, v;

		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction()));

		if (mPlane.intersects(local, pos, u, v))
		{
			collisionPoint.setVertex(PM::pm_SetW(PM::pm_Multiply(matrix(), pos), 1));
			// Do not check flags... calculation is easy anyway.
			collisionPoint.setNormal(PM::pm_RotateWithQuat(rotation(), mPlane.normal()));
			collisionPoint.setUV(PM::pm_Set(u, v));
			return true;
		}

		return false;
	}

	void PlaneEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		if (mMaterial)
		{
			mMaterial->apply(in, this, point, renderer);
		}
	}

	// World space
	FacePoint PlaneEntity::getRandomFacePoint(Random& random) const
	{
		float u = random.getFloat();
		float v = random.getFloat();

		FacePoint fp;
		fp.setVertex(PM::pm_Add(position(),
			PM::pm_Add(PM::pm_Scale(PM::pm_RotateWithQuat(rotation(), PM::pm_Multiply(scale(), mPlane.xAxis())), u),
				PM::pm_Scale(PM::pm_RotateWithQuat(rotation(), PM::pm_Multiply(scale(), mPlane.yAxis())), v))));
		fp.setNormal(PM::pm_RotateWithQuat(rotation(), mPlane.normal()));
		fp.setUV(PM::pm_Set(u, v));
		return fp;
	}
}