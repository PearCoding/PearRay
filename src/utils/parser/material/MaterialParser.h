#pragma once

#include "Config.h"

#include <string>

namespace DL
{
	class DataGroup;
}

namespace PR
{
	class Material;
}

namespace PRU
{
	class Environment;
	class SceneLoader;
	class PR_LIB_UTILS_INLINE MaterialParser
	{
	public:
		virtual PR::Material* parse(SceneLoader* loader, Environment* env,
			const std::string& obj, DL::DataGroup* group) = 0;
	};
}
