#pragma once

#include "Logger.h"
#include <string>

namespace PR {
class PR_LIB_INLINE LogListener {
public:
	virtual void newEntry(Level level, Module m, const std::string& str) = 0;
};
}
