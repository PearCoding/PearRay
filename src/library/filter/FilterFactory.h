#pragma once

#include "PR_Config.h"

namespace PR {
enum FilterType {
	FT_Block = 0,
	FT_Triangle,
	FT_Gaussian,
	FT_Lanczos,
	FT_Mitchell
};

class IFilter;
class PR_LIB FilterFactory {
public:
	static std::shared_ptr<IFilter> createFilter(FilterType type, int radius);
	static std::shared_ptr<IFilter> createFilter(const std::string& type, int radius);
};
} // namespace PR