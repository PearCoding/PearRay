#pragma once

#include "output/OutputSpecification.h"
#include "registry/Registry.h"
#include "renderer/RenderManager.h"
#include "shader/ShadingSocket.h"
#include "spectral/Spectrum.h"

#include <OpenImageIO/texture.h>

#include <list>
#include <map>
#include <utility>

namespace PR {
class IMaterial;
class TriMesh;
class SpectrumDescriptor;

class PR_LIB_UTILS Environment {
public:
	explicit Environment(bool useStandardLib = true);
	virtual ~Environment();

	inline Spectrum getSpectrum(const std::string& name) const
	{
		return mSpectrums.at(name);
	}

	inline bool hasSpectrum(const std::string& name) const
	{
		return mSpectrums.count(name) != 0;
	}

	inline void addSpectrum(const std::string& name, const Spectrum& spec)
	{
		mSpectrums.insert(std::make_pair(name, spec));
	}

	inline std::shared_ptr<IMaterial> getMaterial(const std::string& name) const
	{
		return mMaterials.at(name);
	}

	inline bool hasMaterial(const std::string& name) const
	{
		return mMaterials.count(name) != 0;
	}

	inline void addMaterial(const std::string& name, const std::shared_ptr<IMaterial>& mat)
	{
		PR_ASSERT(mat, "Given material has to be valid");
		PR_ASSERT(!hasMaterial(name), "Given name should be unique");
		mMaterials[name] = mat;
	}

	inline size_t materialCount() const
	{
		return mMaterials.size();
	}

	inline std::shared_ptr<TriMesh> getMesh(const std::string& name) const
	{
		return mMeshes.at(name);
	}

	inline bool hasMesh(const std::string& name) const
	{
		return mMeshes.count(name) != 0;
	}

	inline void addMesh(const std::string& name, const std::shared_ptr<TriMesh>& m)
	{
		PR_ASSERT(m, "Given mesh has to be valid");
		PR_ASSERT(!hasMesh(name), "Given name should be unique");
		mMeshes[name] = m;
	}

	inline void addShadingSocket(const std::string& name,
								const std::shared_ptr<FloatShadingSocket>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!hasShadingSocket(name), "Given name should be unique");
		mNamedShadingSocket[name] = output;
		}

	inline std::shared_ptr<FloatShadingSocket> getShadingSocket(const std::string& name) const
	{
		return mNamedShadingSocket.at(name);
	}

	inline bool hasShadingSocket(const std::string& name) const
	{
		return mNamedShadingSocket.count(name) != 0;
	}

	inline OIIO::TextureSystem* textureSystem()
	{
		return mTextureSystem;
	}

	uint32 renderWidth() const;
	void setRenderWidth(uint32 i);
	uint32 renderHeight() const;
	void setRenderHeight(uint32 i);

	void setCrop(float xmin, float xmax, float ymin, float ymax);
	float cropMinX() const;
	float cropMaxX() const;
	float cropMinY() const;
	float cropMaxY() const;

	inline OutputSpecification& outputSpecification() { return mOutputSpecification; }
	inline const RenderManager& renderManager() const { return mRenderManager; }
	inline RenderManager& renderManager() { return mRenderManager; }

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force = false) const;

private:
	RenderManager mRenderManager;

	std::map<std::string, PR::Spectrum> mSpectrums;
	std::map<std::string, std::shared_ptr<IMaterial>> mMaterials;
	std::map<std::string, std::shared_ptr<TriMesh>> mMeshes;
	std::map<std::string, std::shared_ptr<FloatShadingSocket>> mNamedShadingSocket;

	OIIO::TextureSystem* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR
