#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/IMesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "shader/SamplePoint.h"

#include "math/Projection.h"

#include "Logger.h"

namespace PR
{
	MeshEntity::MeshEntity(const std::string& name, Entity* parent) :
		RenderEntity(name, parent), mMesh(nullptr)
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

	void MeshEntity::setMesh(IMesh* mesh)
	{
		mMesh = mesh;
	}

	IMesh* MeshEntity::mesh() const
	{
		return mMesh;
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

	bool MeshEntity::checkCollision(const Ray& ray, SamplePoint& collisionPoint, float& t)
	{
		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction())));
		
		if (mMesh->checkCollision(local, collisionPoint, t))
		{
			collisionPoint.P = PM::pm_Multiply(matrix(), collisionPoint.P);
			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_RotateWithQuat(rotation(), collisionPoint.Ng));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			t *= scale();

			return true;
		}
		else
		{
			return false;
		}
	}

	SamplePoint MeshEntity::getRandomFacePoint(Sampler& sampler, uint32 sample) const
	{
		SamplePoint point = mMesh->getRandomFacePoint(sampler, sample);
		point.Ng = PM::pm_RotateWithQuat(rotation(), point.Ng);
		point.P = PM::pm_Multiply(matrix(), point.P);
		Projection::tangent_frame(point.Ng, point.Nx, point.Ny);

		return point;
	}
}