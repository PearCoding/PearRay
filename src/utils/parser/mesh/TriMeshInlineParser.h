#pragma once

#include "Config.h"

#include <string>

namespace DL
{
	class DataGroup;
}

namespace PR
{
	class TriMesh;
}

namespace PRU
{
	class SceneLoader;
	class Environment;
	class TriMeshInlineParser
	{
	public:
		PR::TriMesh* parse(SceneLoader* loader, Environment* env, const DL::DataGroup& group) const;
	};
}
