#pragma once

#include "shader/ShadingSocket.h"
#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_UTILS ImageShadingSocket : public PR::FloatShadingSocket {
public:
	ImageShadingSocket(OIIO::TextureSystem* tsys,
					   const OIIO::TextureOpt& options,
					   const std::string& filename);
	vfloat eval(size_t channel, const ShadingPoint& ctx) const override;

private:
	OIIO::ustring mFilename;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;
};
} // namespace PR
