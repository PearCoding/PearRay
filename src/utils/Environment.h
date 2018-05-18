#pragma once

#include "output/OutputSpecification.h"
#include "registry/Registry.h"
#include "scene/SceneFactory.h"
#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"

#include <OpenImageIO/texture.h>

#include <list>
#include <map>
#include <utility>

namespace PR {
class Camera;
class Material;
class TriMesh;
class SpectrumDescriptor;

class PR_LIB_UTILS Environment {
public:
	explicit Environment(const std::shared_ptr<SpectrumDescriptor>& desc, const std::string& name);
	virtual ~Environment();

	inline SceneFactory& sceneFactory()
	{
		return mSceneFactory;
	}

	inline const SceneFactory& sceneFactory() const
	{
		return mSceneFactory;
	}

	inline const std::shared_ptr<SpectrumDescriptor>& spectrumDescriptor() const
	{
		return mSpectrumDescriptor;
	}

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

	inline std::shared_ptr<Material> getMaterial(const std::string& name) const
	{
		return mMaterials.at(name);
	}

	inline bool hasMaterial(const std::string& name) const
	{
		return mMaterials.count(name) != 0;
	}

	inline void addMaterial(const std::string& name, const std::shared_ptr<Material>& mat)
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

	inline void addMesh(const std::string& name, const std::shared_ptr<PR::TriMesh>& m)
	{
		PR_ASSERT(m, "Given mesh has to be valid");
		PR_ASSERT(!hasMesh(name), "Given name should be unique");
		mMeshes[name] = m;
	}

	inline void addShaderOutput(const std::string& name,
								const std::shared_ptr<PR::ScalarShaderOutput>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!hasScalarShaderOutput(name), "Given name should be unique");
		mNamedScalarShaderOutputs[name] = output;
	}

	inline std::shared_ptr<PR::ScalarShaderOutput> getScalarShaderOutput(const std::string& name) const
	{
		return mNamedScalarShaderOutputs.at(name);
	}

	inline bool hasScalarShaderOutput(const std::string& name) const
	{
		return mNamedScalarShaderOutputs.count(name) != 0;
	}

	inline void addShaderOutput(const std::string& name,
								const std::shared_ptr<PR::SpectrumShaderOutput>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!hasSpectrumShaderOutput(name), "Given name should be unique");
		mNamedSpectrumShaderOutputs[name] = output;
	}

	inline std::shared_ptr<PR::SpectrumShaderOutput> getSpectrumShaderOutput(const std::string& name) const
	{
		return mNamedSpectrumShaderOutputs.at(name);
	}

	inline bool hasSpectrumShaderOutput(const std::string& name) const
	{
		return mNamedSpectrumShaderOutputs.count(name) != 0;
	}

	inline void addShaderOutput(const std::string& name,
								const std::shared_ptr<PR::VectorShaderOutput>& output)
	{
		PR_ASSERT(output, "Given output has to be valid");
		PR_ASSERT(!hasVectorShaderOutput(name), "Given name should be unique");
		mNamedVectorShaderOutputs[name] = output;
	}

	inline std::shared_ptr<PR::VectorShaderOutput> getVectorShaderOutput(const std::string& name) const
	{
		return mNamedVectorShaderOutputs.at(name);
	}

	inline bool hasVectorShaderOutput(const std::string& name) const
	{
		return mNamedVectorShaderOutputs.count(name) != 0;
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

	inline OutputSpecification& outputSpecification()
	{
		return mOutputSpecification;
	}

	inline const std::shared_ptr<Registry>& registry() const
	{
		return mRegistry;
	}

	void dumpInformation() const;

	void setup(const std::shared_ptr<RenderContext>& renderer);
	void save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force = false) const;

	std::shared_ptr<RenderFactory> createRenderFactory(const std::string& workingDir) const;
private:
	SceneFactory mSceneFactory;

	std::shared_ptr<Registry> mRegistry;
	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;
	
	std::map<std::string, PR::Spectrum> mSpectrums;
	std::map<std::string, std::shared_ptr<Material>> mMaterials;

	std::map<std::string, std::shared_ptr<TriMesh>> mMeshes;

	std::map<std::string, std::shared_ptr<ScalarShaderOutput>> mNamedScalarShaderOutputs;
	std::map<std::string, std::shared_ptr<SpectrumShaderOutput>> mNamedSpectrumShaderOutputs;
	std::map<std::string, std::shared_ptr<VectorShaderOutput>> mNamedVectorShaderOutputs;

	OIIO::TextureSystem* mTextureSystem;
	OutputSpecification mOutputSpecification;
};
} // namespace PR
