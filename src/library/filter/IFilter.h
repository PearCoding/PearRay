#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB IFilter {
public:
	virtual ~IFilter() = default;

	virtual size_t radius() const					 = 0;
	virtual float evalWeight(float x, float y) const = 0;

	inline size_t width() const { return 2 * radius() + 1; }
	inline size_t height() const { return width(); }
};
} // namespace PR