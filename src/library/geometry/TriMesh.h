#pragma once

#include "IMesh.h"

#include <vector>

namespace PR
{
	class Face;
	class Normal;
	class UV;
	class Vertex;
	class PR_LIB TriMesh : public IMesh
	{
		PR_CLASS_NON_COPYABLE(TriMesh);

	public:
		TriMesh();
		~TriMesh();

		void reserve(size_t count);
		void addFace(Face* f);
		Face* getFace(size_t i) const;
		inline const std::vector<Face*> faces() const
		{
			return mFaces;
		}

		void clear();

		void build();

		void calcNormals();

		// IMesh
		virtual bool isLight() const override;
		inline BoundingBox boundingBox() const override
		{
			return mBoundingBox;
		}

		bool checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t) override;
		FacePoint getRandomFacePoint(Sampler& sampler, uint32 sample) const override;

		virtual void replaceMaterial(Material* mat) override;
	private:
		BoundingBox mBoundingBox;
		void* mKDTree;

		std::vector<Face*> mFaces;
	};
}