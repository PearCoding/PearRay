#pragma once

#include "Logger.h"
#include <string>

namespace PR {
class PR_LIB LogListener {
public:
	LogListener()		   = default;
	virtual ~LogListener() = default;

	virtual void startEntry(LogLevel level) = 0;
	virtual void writeEntry(int c)			= 0;
};
} // namespace PR
