#include "Mesh.h"
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

		for (Face* f : mFaces)
		{
			delete f;
		}
		mFaces.clear();
	}
}