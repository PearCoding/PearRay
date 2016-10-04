#pragma once

#include "BoundingBox.h"

#include <vector>

namespace PR
{
	class Face;
	class Normal;
	class UV;
	class Vertex;
	class Material;
	class Ray;
	class Random;
	struct FaceSample;
	class Sampler;
	class PR_LIB TriMesh
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

		bool isLight() const;
		float surfaceArea(Material* m, const PM::mat& transform) const;

		inline BoundingBox boundingBox() const
		{
			return mBoundingBox;
		}

		float collisionCost() const;

		bool checkCollision(const Ray& ray, FaceSample& collisionPoint);
		FaceSample getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const;

		void replaceMaterial(Material* mat);
	private:
		BoundingBox mBoundingBox;
		void* mKDTree;

		std::vector<Face*> mFaces;
	};
}