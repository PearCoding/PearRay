#pragma once

#include "Config.h"
#include <vector>

namespace PR
{
	class Vertex;
	class Face
	{
	public:
		Face();
		~Face();

		void add(Vertex* v);
		Vertex* get(size_t i) const;

	private:
		std::vector<Vertex*> mVertices;
	};
}