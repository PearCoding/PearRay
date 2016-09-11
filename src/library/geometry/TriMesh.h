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
		void setFaces(const std::vector<Face*>& f);
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
		virtual float surfaceArea(Material* m, const PM::mat& transform) const override;

		inline BoundingBox boundingBox() const override
		{
			return mBoundingBox;
		}

		virtual float collisionCost() const override;

		bool checkCollision(const Ray& ray, FaceSample& collisionPoint, float& t) override;
		FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const override;

		virtual void replaceMaterial(Material* mat) override;
	private:
		BoundingBox mBoundingBox;
		void* mKDTree;

		std::vector<Face*> mFaces;
	};
}