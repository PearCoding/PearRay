#pragma once

#include "Config.h"

#include <string>

namespace DL
{
	class DataGroup;
}

namespace PR
{
	class IMesh;
}

namespace PRU
{
	class Environment;
	class SceneLoader;
	class PR_LIB_UTILS_INLINE IMeshInlineParser
	{
	public:
		virtual PR::IMesh* parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const = 0;
	};
}
