#pragma once

#include "Config.h"
#include <string>

namespace PR
{
	class PR_LIB PerformanceWriter
	{
	public:
		static void write(const std::string& filename);
	};
}