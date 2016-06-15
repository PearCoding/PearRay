#include "PlaneEntity.h"

#include "ray/Ray.h"
#include "material/Material.h"
#include "geometry/FacePoint.h"

#include "Logger.h"
#include "Random.h"
#include "sampler/Sampler.h"

namespace PR
{
	PlaneEntity::PlaneEntity(const std::string& name, const Plane& plane, Entity* parent) :
		RenderEntity(name, parent), mPlane(plane)
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

	bool PlaneEntity::isCollidable() const
	{
		return true;
	}

	BoundingBox PlaneEntity::localBoundingBox() const
	{
		return mPlane.toLocalBoundingBox();
	}

	bool PlaneEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t)
	{
		PM::vec3 pos;
		float u, v;

		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction()));

		if (mPlane.intersects(local, pos, t, u, v))
		{
			collisionPoint.setVertex(PM::pm_SetW(PM::pm_Multiply(matrix(), pos), 1));
			// Do not check flags... calculation is easy anyway.
			collisionPoint.setNormal(PM::pm_RotateWithQuat(rotation(), mPlane.normal()));
			collisionPoint.setUV(PM::pm_Set(u, v));
			t *= scale();

			return true;
		}

		return false;
	}

	// World space
	FacePoint PlaneEntity::getRandomFacePoint(Sampler& sampler, Random& random, uint32 sample) const
	{
		auto s = sampler.generate2D(sample);

		FacePoint fp;
		fp.setVertex(PM::pm_Add(position(),
			PM::pm_Add(PM::pm_Scale(PM::pm_RotateWithQuat(rotation(), PM::pm_Scale(mPlane.xAxis(), scale())), PM::pm_GetX(s)),
				PM::pm_Scale(PM::pm_RotateWithQuat(rotation(), PM::pm_Scale(mPlane.yAxis(), scale())), PM::pm_GetY(s)))));
		fp.setNormal(PM::pm_RotateWithQuat(rotation(), mPlane.normal()));
		fp.setUV(PM::pm_SetZ(s, 0));
		return fp;
	}
}