#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_CORE IFilter {
public:
	virtual ~IFilter() = default;

	virtual int radius() const					 = 0;
	virtual float evalWeight(float x, float y) const = 0;

	inline int width() const { return 2 * radius() + 1; }
	inline int height() const { return width(); }
};
} // namespace PR