#pragma once

#include "PR_Config.h"
#include <filesystem>

namespace PR {
struct ImageSaveOptions {
	bool Parametric = false;
};
class PR_LIB_LOADER ImageIO {
public:
	static bool save(const std::filesystem::path& path,
					 const float* data, size_t width, size_t height, size_t channels,
					 const ImageSaveOptions& opts = ImageSaveOptions());

	static bool load(const std::filesystem::path& path,
					 std::vector<float>& data, size_t& width, size_t& height, size_t& channels);
};
} // namespace PR