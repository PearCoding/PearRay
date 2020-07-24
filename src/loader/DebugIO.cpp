#include "DebugIO.h"
#include "config/Build.h"

#include <OpenImageIO/imageio.h>

namespace PR {
namespace Debug {
void saveImage(const std::string& path, const float* data, size_t width, size_t height, size_t channels)
{
	OIIO::ImageSpec spec(width, height, channels, OIIO::TypeDesc::FLOAT);

	const std::string versionStr = Build::getVersionString();
	spec.attribute("Software", "PearRay " + versionStr);
	spec.attribute("IPTC:ProgramVersion", versionStr);

	auto out = OIIO::ImageOutput::create(path);
	if (!out)
		return;

	if (!out->open(path, spec)) {
#if OIIO_PLUGIN_VERSION < 22
		OIIO::ImageOutput::destroy(out);
#endif
		return;
	}

	out->write_image(OIIO::TypeDesc::FLOAT, data);
	out->close();

#if OIIO_PLUGIN_VERSION < 22
	OIIO::ImageOutput::destroy(out);
#endif
}
} // namespace Debug
} // namespace PR