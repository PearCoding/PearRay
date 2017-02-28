#pragma once

#include "PR_Config.h"

#include <string>

namespace DL
{
	class DataGroup;
}

namespace PR
{
	class Entity;
}

namespace PRU
{
	class Environment;
	class SceneLoader;
	class PR_LIB_UTILS_INLINE IEntityParser
	{
	public:
		virtual std::shared_ptr<PR::Entity> parse(SceneLoader* loader, Environment* env, const std::string& name,
			const std::string& obj, const DL::DataGroup& group) const = 0;
	};
}
