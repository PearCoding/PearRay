#pragma once

#include "shader/INode.h"

#include <OpenImageIO/texture.h>

namespace PR {
class SpectralUpsampler;

class PR_LIB_LOADER ParametricImageNode : public FloatSpectralNode {
public:
	ParametricImageNode(OIIO::TextureSystem* tsys,
						const OIIO::TextureOpt& options,
						const std::string& filename);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	OIIO::ustring mFilename;
	void* mHandle;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;

	bool mIsPtex;

	mutable std::atomic<bool> mErrorIdenticator;
};

/// Do not use this, its just a workaround as saving parametric values into a file is broken currently...
class PR_LIB_LOADER NonParametricImageNode : public FloatSpectralNode {
public:
	NonParametricImageNode(OIIO::TextureSystem* tsys,
						   const OIIO::TextureOpt& options,
						   const std::string& filename,
						   SpectralUpsampler* upsampler);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	OIIO::ustring mFilename;
	void* mHandle;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;
	SpectralUpsampler* mUpsampler;

	bool mIsPtex;

	mutable std::atomic<bool> mErrorIdenticator;
};

class PR_LIB_LOADER ScalarImageNode : public FloatScalarNode {
public:
	ScalarImageNode(OIIO::TextureSystem* tsys,
					const OIIO::TextureOpt& options,
					const std::string& filename);
	float eval(const ShadingContext& ctx) const override;
	//Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	OIIO::ustring mFilename;
	void* mHandle;
	OIIO::TextureOpt mTextureOptions;
	OIIO::TextureSystem* mTextureSystem;

	bool mIsPtex;

	mutable std::atomic<bool> mErrorIdenticator;
};
} // namespace PR
