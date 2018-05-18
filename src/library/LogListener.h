#pragma once

#include "Logger.h"
#include <string>

namespace PR {
class PR_LIB_INLINE LogListener {
public:
	virtual void startEntry(LogLevel level) = 0;
	virtual void writeEntry(int c) = 0;
};
}
