#pragma once

#include "shader/Node.h"

#include <OpenImageIO/texture.h>

namespace PR {
class PR_LIB_LOADER ImageNode : public PR::FloatSpectralNode {
public:
	ImageNode(OIIO::TextureSystem* tsys,
				   const OIIO::TextureOpt& options,
				   const std::string& filename);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	void lookup(const ShadingContext& x, SpectralBlob& rgb) const;

	OIIO::ustring mFilename;
	void* mHandle;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;

	bool mIsPtex;
};
} // namespace PR
