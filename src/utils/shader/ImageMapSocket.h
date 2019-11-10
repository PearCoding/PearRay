#pragma once

#include "shader/Socket.h"

#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_UTILS ImageMapSocket : public PR::FloatSpectralMapSocket {
public:
	ImageMapSocket(OIIO::TextureSystem* tsys,
				   const OIIO::TextureOpt& options,
				   const std::string& filename);
	float eval(const MapSocketCoord& ctx) const override;
	float relativeLuminance(const MapSocketCoord& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	OIIO::ustring mFilename;
	void* mHandle;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;

	bool mIsPtex;
};
} // namespace PR
