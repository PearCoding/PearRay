#pragma once

#include "Config.h"
#include <string>

namespace PR
{
	class Mesh;
}

namespace PRU
{
	class PR_LIB_UTILS MeshLoader
	{
	public:
		virtual void load(const std::string& file, PR::Mesh* mesh) = 0;
	};
}
