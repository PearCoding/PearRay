#include "Mesh.h"
#include "Normal.h"
#include "UV.h"
#include "Vertex.h"
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

	void Mesh::addVertex(Vertex* v)
	{
		PR_ASSERT(v);
		mVertices.push_back(v);
	}

	Vertex* Mesh::getVertex(size_t i) const
	{
		return mVertices.at(i);
	}

	void Mesh::addNormal(Normal* v)
	{
		PR_ASSERT(v);
		mNormals.push_back(v);
	}

	Normal* Mesh::getNormal(size_t i) const
	{
		return mNormals.at(i);
	}

	void Mesh::addUV(UV* v)
	{
		PR_ASSERT(v);
		mUVs.push_back(v);
	}

	UV* Mesh::getUV(size_t i) const
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
		for (Vertex* v : mVertices)
		{
			delete v;
		}
		mVertices.clear();

		for (Normal* v : mNormals)
		{
			delete v;
		}
		mNormals.clear();

		for (UV* v : mUVs)
		{
			delete v;
		}
		mUVs.clear();

		for (Face* f : mFaces)
		{
			delete f;
		}
		mFaces.clear();
	}
}