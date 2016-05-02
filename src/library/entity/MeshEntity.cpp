#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/Mesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "geometry/FacePoint.h"

#include "Random.h"
#include "sampler/Sampler.h"
#include "sampler/Projection.h"

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

	void MeshEntity::setMesh(Mesh* mesh)
	{
		mMesh = mesh;
	}

	Mesh* MeshEntity::mesh() const
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

	bool MeshEntity::checkCollision(const Ray& ray, FacePoint& collisionPoint)
	{
		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_RotateWithQuat(PM::pm_InverseQuat(rotation()), ray.direction()));

		float near = std::numeric_limits<float>::max();
		bool found = false;

		for (Face* face : mMesh->faces())// TODO: For bigger meshes use voxel technique!
		{
			float u, v;
			PM::vec3 point;
			
			if (Triangle::intersect(local, face->V1, face->V2, face->V3, u, v, point))
			{
				float mag = PM::pm_Magnitude3D(PM::pm_Subtract(point, local.startPosition()));

				if (near > mag)
				{
					PM::vec3 vec;
					PM::vec3 n;
					PM::vec2 uv;
					face->interpolate(u, v, vec, n, uv);

					collisionPoint.setVertex(point);
					collisionPoint.setNormal(n);
					collisionPoint.setUV(uv);

					near = mag;
					found = true;
				}
			}
		}

		collisionPoint.setVertex(PM::pm_Multiply(matrix(), collisionPoint.vertex()));
		collisionPoint.setNormal(PM::pm_RotateWithQuat(rotation(), collisionPoint.normal()));

		return true;
	}

	FacePoint MeshEntity::getRandomFacePoint(Sampler& sampler, Random& random) const
	{
		auto ret = sampler.generate(random);
		uint32 fi = Projection::map(PM::pm_GetX(ret), 0, mMesh->faces().size() - 1);

		Face* face = mMesh->getFace(fi);

		PM::vec3 vec;
		PM::vec3 n;
		PM::vec2 uv;
		face->interpolate(PM::pm_GetY(ret), PM::pm_GetZ(ret), vec, n, uv);
		
		vec = PM::pm_Multiply(matrix(), vec);
		n = PM::pm_RotateWithQuat(rotation(), n);


		FacePoint fp;
		fp.setVertex(vec);
		fp.setNormal(n);
		fp.setUV(uv);

		return fp;
	}
}