#pragma once

#include "scene/Scene.h"
#include "spectral/Spectrum.h"
#include "shader/ShaderOutput.h"
#include "output/OutputSpecification.h"

#include <OpenImageIO/texture.h>

#include <map>
#include <list>

namespace PR
{
	class Camera;
	class Material;
	class TriMesh;
	
	class PR_LIB_UTILS Environment
	{
	public:
		explicit Environment(const std::string& name);
		virtual ~Environment();

		inline Scene& scene()
		{
			return mScene;
		}

		inline const Scene& scene() const
		{
			return mScene;
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
			mSpectrums[name] = spec;
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
			const std::shared_ptr<PR::SpectralShaderOutput>& output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			PR_ASSERT(!hasSpectralShaderOutput(name), "Given name should be unique");
			mNamedSpectralShaderOutputs[name] = output;
		}

		inline std::shared_ptr<PR::SpectralShaderOutput> getSpectralShaderOutput(const std::string& name) const
		{
			return mNamedSpectralShaderOutputs.at(name);
		}

		inline bool hasSpectralShaderOutput(const std::string& name) const
		{
			return mNamedSpectralShaderOutputs.count(name) != 0;
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

		inline PR::Camera* camera() const
		{
			return mCamera;
		}

		inline void setCamera(Camera* cam)
		{
			mCamera = cam;
		}

		inline uint32 renderWidth() const
		{
			return mRenderWidth;
		}

		inline void setRenderWidth(uint32 i)
		{
			mRenderWidth = i;
		}

		inline uint32 renderHeight() const
		{
			return mRenderHeight;
		}

		inline void setRenderHeight(uint32 i)
		{
			mRenderHeight = i;
		}

		inline void setCrop(float xmin, float xmax, float ymin, float ymax)
		{
			mCropMinX = xmin;
			mCropMaxX = xmax;
			mCropMinY = ymin;
			mCropMaxY = ymax;
		}

		inline float cropMinX() const
		{
			return mCropMinX;
		}

		inline float cropMaxX() const
		{
			return mCropMaxX;
		}

		inline float cropMinY() const
		{
			return mCropMinY;
		}

		inline float cropMaxY() const
		{
			return mCropMaxY;
		}

		inline OutputSpecification& outputSpecification()
		{
			return mOutputSpecification;
		}

		void dumpInformation() const;
	private:
		Scene mScene;
		Camera* mCamera;
		uint32 mRenderWidth;
		uint32 mRenderHeight;

		float mCropMinX;
		float mCropMaxX;
		float mCropMinY;
		float mCropMaxY;

		std::map<std::string, PR::Spectrum> mSpectrums;
		std::map<std::string, std::shared_ptr<Material> > mMaterials;

		std::map<std::string, std::shared_ptr<TriMesh> > mMeshes;

		std::map<std::string, std::shared_ptr<ScalarShaderOutput> > mNamedScalarShaderOutputs;
		std::map<std::string, std::shared_ptr<SpectralShaderOutput> > mNamedSpectralShaderOutputs;
		std::map<std::string, std::shared_ptr<VectorShaderOutput> > mNamedVectorShaderOutputs;

		OIIO::TextureSystem* mTextureSystem;
		OutputSpecification mOutputSpecification;
	};
}
