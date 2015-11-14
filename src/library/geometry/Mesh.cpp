#include "Mesh.h"
#include "Face.h"

namespace PR
{
	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{
		clear();
	}

	void Mesh::addVertex(const PM::vec3& v)
	{
		mBoundingBox.put(v);// FIXME: 0 is already in :/
		mVertices.push_back(v);
	}

	PM::vec3 Mesh::getVertex(size_t i) const
	{
		return mVertices.at(i);
	}

	void Mesh::addNormal(const PM::vec3& v)
	{
		mNormals.push_back(v);
	}

	PM::vec3 Mesh::getNormal(size_t i) const
	{
		return mNormals.at(i);
	}

	void Mesh::addUV(const PM::vec2& v)
	{
		mUVs.push_back(v);
	}

	PM::vec2 Mesh::getUV(size_t i) const
	{
		return mUVs.at(i);
	}


	void Mesh::addFace(Face* f)
	{
		PR_ASSERT(f);
		mFaces.push_back(f);
	}

	Face* Mesh::getFace(size_t i) const
	{
		return mFaces.at(i);
	}

	void Mesh::clear()
	{
		mVertices.clear();
		mNormals.clear();
		mUVs.clear();

		for (Face* f : mFaces)
		{
			delete f;
		}
		mFaces.clear();
	}
}