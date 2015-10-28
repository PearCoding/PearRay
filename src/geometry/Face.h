#pragma once

#include "Config.h"
#include <vector>

namespace PR
{
	class Normal;
	class UV;
	class Vertex;
	class Face
	{
	public:
		Face();
		~Face();

		void addVertex(Vertex* v);
		Vertex* getVertex(size_t i) const;

		void addNormal(Normal* v);
		Normal* getNormal(size_t i) const;

		void addUV(UV* v);
		UV* getUV(size_t i) const;

	private:
		std::vector<Vertex*> mVertices;
		std::vector<Normal*> mNormals;
		std::vector<UV*> mUVs;
	};
}