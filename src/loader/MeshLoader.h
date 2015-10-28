#pragma once

#include "Config.h"
#include <string>

namespace PR
{
	class Mesh;
	class MeshLoader
	{
	public:
		virtual void load(const std::string& file, Mesh* mesh) = 0;
	};
}
