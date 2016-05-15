#pragma once

#include "BoundingBox.h"

#include <vector>

namespace PR
{
	class Face;
	class Normal;
	class UV;
	class Vertex;
	class PR_LIB Mesh
	{
		PR_CLASS_NON_COPYABLE(Mesh);

	public:
		Mesh();
		~Mesh();

		inline BoundingBox boundingBox() const
		{
			return mBoundingBox;
		}

		void addVertex(const PM::vec3& v);
		PM::vec3 getVertex(size_t i) const;
		inline const std::vector<PM::vec3>& vertices() const
		{
			return mVertices;
		}

		void addNormal(const PM::vec3& v);
		PM::vec3 getNormal(size_t i) const;
		inline const std::vector<PM::vec3>& normals() const
		{
			return mNormals;
		}

		void addUV(const PM::vec2& v);
		PM::vec2 getUV(size_t i) const;
		inline const std::vector<PM::vec2>& uvs() const
		{
			return mUVs;
		}

		void addFace(Face* f);
		Face* getFace(size_t i) const;
		inline const std::vector<Face*>& faces() const
		{
			return mFaces;
		}

		void clear();

		void build();
		inline void* kdTree() const
		{
			return mKDTree;
		}

		void fix();// Produce Normals and UV coords if needed.
	private:
		BoundingBox mBoundingBox;
		void* mKDTree;

		std::vector<PM::vec3> mVertices;
		std::vector<PM::vec3> mNormals;
		std::vector<PM::vec2> mUVs;
		std::vector<Face*> mFaces;
	};
}