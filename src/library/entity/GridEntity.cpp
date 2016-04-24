#include "GridEntity.h"

#include "material/Material.h"
#include "geometry/FacePoint.h"

#include "Logger.h"
#include "Random.h"

namespace PR
{
	GridEntity::GridEntity(const std::string& name, const Plane& plane, Entity* parent) :
		RenderEntity(name, parent), mPlane(plane), mFirstMaterial(nullptr), mSecondMaterial(nullptr), mGridCount(10)
	{
	}

	GridEntity::~GridEntity()
	{
	}

	std::string GridEntity::type() const
	{
		return "grid";
	}

	void GridEntity::setPlane(const Plane& plane)
	{
		mPlane = plane;
	}

	Plane GridEntity::plane() const
	{
		return mPlane;
	}

	void GridEntity::setGridCount(int f)
	{
		mGridCount = f;
	}

	int GridEntity::gridCount() const
	{
		return mGridCount;
	}

	bool GridEntity::isLight() const
	{
		return (mFirstMaterial ? mFirstMaterial->isLight() : false) ||
			(mSecondMaterial ? mSecondMaterial->isLight() : false);
	}

	void GridEntity::setFirstMaterial(Material* m)
	{
		mFirstMaterial = m;
	}

	Material* GridEntity::firstMaterial() const
	{
		return mFirstMaterial;
	}

	void GridEntity::setSecondMaterial(Material* m)
	{
		mSecondMaterial = m;
	}

	Material* GridEntity::secondMaterial() const
	{
		return mSecondMaterial;
	}

	bool GridEntity::isCollidable() const
	{
		return true;
	}

	BoundingBox GridEntity::localBoundingBox() const
	{
		return mPlane.toLocalBoundingBox();
	}

	bool GridEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		mPlane.setPosition(position());// Update

		PM::vec3 pos;
		float u, v;

		if (mPlane.intersects(ray, pos, u, v))
		{
			collisionPoint.setVertex(pos);
			// Do not check flags... calculation is easy anyway.
			collisionPoint.setNormal(mPlane.normal());
			collisionPoint.setUV(PM::pm_Set(u, v));
			return true;
		}

		return false;
	}

	void GridEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		Material* m = nullptr;

		int du = (int) (PM::pm_GetX(point.uv()) * mGridCount);
		int dv = (int) (PM::pm_GetY(point.uv()) * mGridCount);

		if (du % 2 == dv % 2)
		{
			m = mFirstMaterial;
		}
		else
		{
			m = mSecondMaterial;
		}

		if (m)
		{
			m->apply(in, this, point, renderer);
		}
	}

	FacePoint GridEntity::getRandomFacePoint(Random& random) const
	{
		float u = random.getFloat();
		float v = random.getFloat();

		FacePoint fp;
		fp.setVertex(PM::pm_Add(position(), PM::pm_Add(PM::pm_Scale(mPlane.xAxis(), u), PM::pm_Scale(mPlane.yAxis(), v))));
		fp.setNormal(mPlane.normal());
		fp.setUV(PM::pm_Set(u, v));
		return fp;
	}
}