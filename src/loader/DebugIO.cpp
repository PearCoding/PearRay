#include "DebugIO.h"
#include "ImageIO.h"

namespace PR {
namespace Debug {
void saveImage(const std::string& path, const float* data, size_t width, size_t height, size_t channels)
{
	ImageIO::save(path, data, width, height, channels);
}
} // namespace Debug
} // namespace PR