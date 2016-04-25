#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>

namespace PR
{
	class Renderer;
}

namespace PRU
{
	class Environment;
	class PR_LIB_UTILS RenderLoader
	{
	public:
		RenderLoader();
		virtual ~RenderLoader();

		PR::Renderer* loadFromFile(Environment* env, const std::string& path);
		PR::Renderer* load(Environment* env, const std::string& source);
	};
}
