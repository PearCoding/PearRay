#pragma once

#include <string>
#include <vector>

namespace PR {
void save_normalized_gray(const std::string& path, size_t xres, size_t yres, const std::vector<float>& data);
void save_gray(const std::string& path, size_t xres, size_t yres, const std::vector<float>& data);
void save_image(const std::string& path, size_t xres, size_t yres, const std::vector<float>& data, size_t channels=3);
void save_color_preview(const std::string& path,
						float r, float g, float b,
						float r2, float g2, float b2);
} // namespace PR