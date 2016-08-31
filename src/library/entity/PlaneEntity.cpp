#include "PlaneEntity.h"

#include "ray/Ray.h"
#include "material/Material.h"
#include "shader/SamplePoint.h"

#include "Logger.h"
#include "Random.h"
#include "sampler/Sampler.h"
#include "math/Projection.h"

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

	float PlaneEntity::collisionCost() const
	{
		return 2;
	}

	BoundingBox PlaneEntity::localBoundingBox() const
	{
		return mPlane.toLocalBoundingBox();
	}

	bool PlaneEntity::checkCollision(const Ray& ray, SamplePoint& collisionPoint)
	{
		PM::vec3 pos;
		float u, v;

		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Transform(worldInvMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Multiply(worldInvDirectionMatrix(), ray.direction())));

		float t;
		if (mPlane.intersects(local, pos, t, u, v))
		{
			collisionPoint.P = PM::pm_Transform(worldMatrix(), pos);

			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Multiply(worldDirectionMatrix(), mPlane.normal()));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			collisionPoint.UV = PM::pm_Set(u, v);
			collisionPoint.Material = material();

			return true;
		}

		return false;
	}

	// World space
	SamplePoint PlaneEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		auto s = sampler.generate2D(sample);

		PM::vec xaxis = PM::pm_Multiply(worldDirectionMatrix(), mPlane.xAxis());
		PM::vec yaxis = PM::pm_Multiply(worldDirectionMatrix(), mPlane.yAxis());

		SamplePoint fp;
		fp.P = PM::pm_Add(worldPosition(),
			PM::pm_Add(PM::pm_Scale(xaxis, PM::pm_GetX(s)),
				PM::pm_Scale(yaxis, PM::pm_GetY(s))));
		fp.Ng = PM::pm_Normalize3D(PM::pm_Multiply(worldDirectionMatrix(), mPlane.normal()));
		fp.N = fp.Ng;
		Projection::tangent_frame(fp.Ng, fp.Nx, fp.Ny);

		fp.UV = PM::pm_SetZ(s, 0);
		fp.Material = material();

		return fp;
	}
}