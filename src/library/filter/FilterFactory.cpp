#include "FilterFactory.h"
#include "BlockFilter.h"
#include "GaussianFilter.h"
#include "LanczosFilter.h"
#include "MitchellFilter.h"
#include "TriangleFilter.h"

namespace PR {
std::shared_ptr<IFilter> FilterFactory::createFilter(FilterType type, int radius)
{
	switch (type) {
	default:
	case FT_Block:
		return std::make_shared<BlockFilter>(radius);
	case FT_Triangle:
		return std::make_shared<TriangleFilter>(radius);
	case FT_Gaussian:
		return std::make_shared<GaussianFilter>(radius);
	case FT_Lanczos:
		return std::make_shared<LanczosFilter>(radius);
	case FT_Mitchell:
		return std::make_shared<MitchellFilter>(radius);
	}
}

static struct {
	const char* Name;
	FilterType Type;
} _s_types[] = {
	{ "default", FT_Block },
	{ "standard", FT_Block },
	{ "block", FT_Block },
	{ "const", FT_Block },
	{ "constant", FT_Block },
	{ "triangle", FT_Triangle },
	{ "gaussian", FT_Gaussian },
	{ "gauss", FT_Gaussian },
	{ "blur", FT_Gaussian },
	{ "lanczos", FT_Lanczos },
	{ "sinc", FT_Lanczos },
	{ "mitchell", FT_Mitchell },
	{ nullptr, FT_Block }
};

std::shared_ptr<IFilter> FilterFactory::createFilter(const std::string& type, int radius)
{
	std::string name = type;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	for (int i = 0; _s_types[i].Name; ++i) {
		if (name == _s_types[i].Name)
			return createFilter(_s_types[i].Type, radius);
	}

	return createFilter(FT_Block, radius);
}
} // namespace PR