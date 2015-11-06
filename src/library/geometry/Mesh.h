#pragma once

#include "Config.h"
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

		void addVertex(Vertex* v);
		Vertex* getVertex(size_t i) const;

		void addNormal(Normal* v);
		Normal* getNormal(size_t i) const;

		void addUV(UV* v);
		UV* getUV(size_t i) const;

		void addFace(Face* f);
		Face* getFace(size_t i) const;

		void clear();
	private:
		std::vector<Vertex*> mVertices;
		std::vector<Normal*> mNormals;
		std::vector<UV*> mUVs;
		std::vector<Face*> mFaces;
	};
}