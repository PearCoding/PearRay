#pragma once

#include "Config.h"

#include <string>

namespace DL
{
	class DataGroup;
}

namespace PRU
{
	class Environment;
	class SceneLoader;
	class PR_LIB_UTILS TextureParser
	{
	public:
		void parse(SceneLoader* loader, Environment* env,
			const std::string& name, DL::DataGroup* group) const;
	};
}
