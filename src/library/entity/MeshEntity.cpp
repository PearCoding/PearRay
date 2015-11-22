#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/Mesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "geometry/FacePoint.h"

#include "Random.h"

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

	bool MeshEntity::isLight() const
	{
		return mMaterial ? mMaterial->isLight() : false;
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
		if (!worldBoundingBox().intersects(ray))
		{
			return false;
		}

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
					PM::vec3 vec;
					PM::vec3 n;
					PM::vec2 uv;
					face->interpolate(u, v, vec, n, uv);
					n = PM::pm_RotateWithQuat(rotation(), n);

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

	FacePoint MeshEntity::getRandomFacePoint(Random& random) const
	{
		uint32 fi = random.get32(0, mMesh->faces().size());

		Face* face = mMesh->getFace(fi);

		float u = random.getFloat();
		float v = random.getFloat();

		PM::vec3 vec;
		PM::vec3 n;
		PM::vec2 uv;
		face->interpolate(u, v, vec, n, uv);
		
		vec = PM::pm_Multiply(matrix(), vec);
		n = PM::pm_RotateWithQuat(rotation(), n);


		FacePoint fp;
		fp.setVertex(vec);
		fp.setNormal(n);
		fp.setUV(uv);

		return fp;
	}
}