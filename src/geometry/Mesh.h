#pragma once

#include "Config.h"
#include <vector>

namespace PR
{
	class Face;
	class Vertex;
	class Mesh
	{
		PR_CLASS_NON_COPYABLE(Mesh);

	public:
		Mesh();
		~Mesh();

		void addVertex(Vertex* v);
		Vertex* getVertex(size_t i) const;

		void addFace(Face* f);
		Face* getFace(size_t i) const;

		void clear();
	private:
		std::vector<Vertex*> mVertices;
		std::vector<Face*> mFaces;
	};
}