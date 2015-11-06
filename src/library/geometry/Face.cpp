#include "Face.h"

namespace PR
{
	Face::Face()
	{
	}

	Face::~Face()
	{
	}

	void Face::addVertex(Vertex* v)
	{
		PR_ASSERT(v);
		mVertices.push_back(v);
	}

	Vertex* Face::getVertex(size_t i) const
	{
		return mVertices.at(i);
	}

	void Face::addNormal(Normal* v)
	{
		PR_ASSERT(v);
		mNormals.push_back(v);
	}

	Normal* Face::getNormal(size_t i) const
	{
		return mNormals.at(i);
	}

	void Face::addUV(UV* v)
	{
		PR_ASSERT(v);
		mUVs.push_back(v);
	}

	UV* Face::getUV(size_t i) const
	{
		return mUVs.at(i);
	}
}