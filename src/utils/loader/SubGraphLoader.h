#pragma once

#include "Config.h"
#include <string>

namespace PR
{
	class IMesh;
}

namespace PRU
{
	class Environment;
	class PR_LIB_UTILS SubGraphLoader
	{
	public:
		virtual void load(const std::string& file, Environment* env) = 0;
	};
}
