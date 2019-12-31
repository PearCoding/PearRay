#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB ICachable {
	friend class Cache;

public:
	virtual bool isLoaded() const	  = 0;
	virtual size_t memoryUsage() const = 0;
	virtual size_t accessCount() const = 0;

protected:
	virtual void load()   = 0;
	virtual void unload() = 0;
};
} // namespace PR