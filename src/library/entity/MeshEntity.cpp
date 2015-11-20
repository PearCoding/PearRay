#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/Mesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "geometry/FacePoint.h"

#include "Logger.h"

namespace PR
{
	MeshEntity::MeshEntity(const std::string& name, Entity* parent) :
		GeometryEntity(name, parent), mMesh(nullptr)
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

	void MeshEntity::setMaterial(Material* m)
	{
		mMaterial = m;
	}

	Material* MeshEntity::material() const
	{
		return mMaterial;
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
		float near = std::numeric_limits<float>::max();
		bool found = false;

		PM::mat rot = PM::pm_Rotation(rotation());
		PM::mat mat = matrix();
		for (Face* face : mMesh->faces())// TODO: For bigger meshes use voxel technique!
		{
			float u, v;
			PM::vec3 point;

			PM::vec3 p1 = PM::pm_Multiply(mat, face->V1);
			PM::vec3 p2 = PM::pm_Multiply(mat, face->V2);
			PM::vec3 p3 = PM::pm_Multiply(mat, face->V3);

			if (Triangle::intersect(ray, p1, p2, p3, u, v, point))
			{
				float mag = PM::pm_Magnitude3D(PM::pm_Subtract(point, ray.startPosition()));

				if (near > mag)
				{
					PM::vec3 n = PM::pm_Add(PM::pm_Scale(face->N2, u),
						PM::pm_Add(PM::pm_Scale(face->N3, v), PM::pm_Scale(face->N1, 1 - u - v)));
					n = PM::pm_Multiply(rot, n);

					PM::vec2 uv = PM::pm_Add(PM::pm_Scale(face->UV2, u),
						PM::pm_Add(PM::pm_Scale(face->UV3, v), PM::pm_Scale(face->UV1, 1 - u - v)));

					collisionPoint.setVertex(point);
					collisionPoint.setNormal(n);
					collisionPoint.setUV(uv);

					near = mag;
					found = true;
				}
			}
		}

		return found;
	}

	void MeshEntity::apply(Ray& in, const FacePoint& point, Renderer* renderer)
	{
		if (mMaterial)
		{
			mMaterial->apply(in, this, point, renderer);
		}
	}
}