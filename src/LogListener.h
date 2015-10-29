#pragma once

#include "Logger.h"
#include <string>

namespace PR
{
	class LogListener
	{
	public:
		virtual void newEntry(Level level, Module m, const std::string& str) = 0;
	};
}
