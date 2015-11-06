#pragma once

#include "Config.h"
#include <string>

namespace PR
{
	class Mesh;
	class PR_LIB MeshLoader
	{
	public:
		virtual void load(const std::string& file, Mesh* mesh) = 0;
	};
}
