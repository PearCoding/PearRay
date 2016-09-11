#pragma once

#include "Config.h"

#include <string>

namespace DL
{
	class DataGroup;
}

namespace PR
{
	class IInfiniteLight;
}

namespace PRU
{
	class Environment;
	class SceneLoader;
	class PR_LIB_UTILS_INLINE ILightParser
	{
	public:
		virtual PR::IInfiniteLight* parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const = 0;
	};
}
