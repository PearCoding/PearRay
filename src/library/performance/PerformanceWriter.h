#pragma once

#include "PR_Config.h"
#include <string>

namespace PR
{
	class PR_LIB PerformanceWriter
	{
	public:
		static void write(const std::string& filename);
	};
}
