#include "Face.h"

namespace PR
{
	Face::Face()
	{
	}

	Face::~Face()
	{
	}

	void Face::add(Vertex* v)
	{
		PR_ASSERT(v);
		mVertices.push_back(v);
	}

	Vertex* Face::get(size_t i) const
	{
		return mVertices.at(i);
	}
}