#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/IMesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "geometry/FacePoint.h"

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

	BoundingBox MeshEntity::localBoundingBox() const
	{
		PR_DEBUG_ASSERT(mMesh);
		return mMesh->boundingBox();
	}

	bool MeshEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t)
	{
		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction())));
		
		if (mMesh->checkCollision(local, collisionPoint, t))
		{
			collisionPoint.setVertex(PM::pm_Multiply(matrix(), collisionPoint.vertex()));
			collisionPoint.setNormal(PM::pm_Normalize3D(PM::pm_RotateWithQuat(rotation(), collisionPoint.normal())));
			t *= scale();

			return true;
		}
		else
		{
			return false;
		}
	}

	FacePoint MeshEntity::getRandomFacePoint(Sampler& sampler, Random& random, uint32 sample) const
	{
		FacePoint point = mMesh->getRandomFacePoint(sampler, random, sample);
		point.setNormal(PM::pm_RotateWithQuat(rotation(), point.normal()));
		point.setVertex(PM::pm_Multiply(matrix(), point.vertex()));
		return point;
	}
}