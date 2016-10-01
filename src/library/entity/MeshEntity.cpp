#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/IMesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "shader/FaceSample.h"

#include "math/Projection.h"

#include "Logger.h"

#include "performance/Performance.h"

namespace PR
{
	MeshEntity::MeshEntity(const std::string& name) :
		RenderEntity(name), mMesh(nullptr), mMaterialOverride(nullptr)
	{
	}

	MeshEntity::~MeshEntity()
	{
	}

	std::string MeshEntity::type() const
	{
		return "mesh";
	}

	bool MeshEntity::isLight() const
	{
		return mMesh->isLight();
	}

	float MeshEntity::surfaceArea(Material* m) const
	{
		if(isFrozen() && (m == nullptr || m == mMaterialOverride))
			return mSurfaceArea_Cache;
		else
			return mMesh->surfaceArea(m, flags() & EF_LocalArea ? PM::pm_Identity() : matrix());
	}

	void MeshEntity::setMesh(IMesh* mesh)
	{
		mMesh = mesh;
	}

	IMesh* MeshEntity::mesh() const
	{
		return mMesh;
	}

	void MeshEntity::setMaterialOverride(Material* m)
	{
		mMaterialOverride = m;
	}

	Material* MeshEntity::materialOverride() const
	{
		return mMaterialOverride;
	}

	bool MeshEntity::isCollidable() const
	{
		return mMesh != nullptr;
	}

	float MeshEntity::collisionCost() const
	{
		return mMesh->collisionCost();
	}

	BoundingBox MeshEntity::localBoundingBox() const
	{
		PR_DEBUG_ASSERT(mMesh);
		return mMesh->boundingBox();
	}

	bool MeshEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_GUARD_PROFILE();

		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Transform(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Transform(invDirectionMatrix(), ray.direction())));
		
		if (mMesh->checkCollision(local, collisionPoint))
		{
			collisionPoint.P = PM::pm_Transform(matrix(), collisionPoint.P);
			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Transform(directionMatrix(), collisionPoint.Ng));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			if(mMaterialOverride)
				collisionPoint.Material = mMaterialOverride;

			return true;
		}
		else
		{
			return false;
		}
	}

	FaceSample MeshEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		PR_GUARD_PROFILE();

		FaceSample point = mMesh->getRandomFacePoint(sampler, sample, pdf);
		point.Ng = PM::pm_Normalize3D(PM::pm_Transform(directionMatrix(), point.Ng));
		point.P = PM::pm_Transform(matrix(), point.P);
		Projection::tangent_frame(point.Ng, point.Nx, point.Ny);

		return point;
	}

	void MeshEntity::onFreeze()
	{
		RenderEntity::onFreeze();

		mSurfaceArea_Cache = mMesh->surfaceArea(nullptr,
				flags() & EF_LocalArea ? PM::pm_Identity() : matrix());
	}
}