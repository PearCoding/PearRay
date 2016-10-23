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
}

namespace PRU
{
	class PR_LIB_UTILS Environment
	{
	public:
		explicit Environment(const std::string& name);
		virtual ~Environment();

		inline PR::Scene* scene()
		{
			return &mScene;
		}

		inline PR::Spectrum getSpectrum(const std::string& name) const
		{
			return mSpectrums.at(name);
		}

		inline bool hasSpectrum(const std::string& name) const
		{
			return mSpectrums.count(name) != 0;
		}

		inline void addSpectrum(const std::string& name, const PR::Spectrum& spec)
		{
			mSpectrums[name] = spec;
		}

		inline PR::Material* getMaterial(const std::string& name) const
		{
			return mMaterials.at(name);
		}

		inline bool hasMaterial(const std::string& name) const
		{
			return mMaterials.count(name) != 0;
		}

		inline void addMaterial(const std::string& name, PR::Material* mat)
		{
			PR_ASSERT(mat && !hasMaterial(name));
			mMaterials[name] = mat;
		}

		inline size_t materialCount() const
		{
			return mMaterials.size();
		}

		inline PR::TriMesh* getMesh(const std::string& name) const
		{
			return mMeshes.at(name);
		}

		inline bool hasMesh(const std::string& name) const
		{
			return mMeshes.count(name) != 0;
		}

		inline void addMesh(const std::string& name, PR::TriMesh* m)
		{
			PR_ASSERT(m && !hasMesh(name));
			mMeshes[name] = m;
		}

		inline void addShaderOutput(const std::string& name, PR::ScalarShaderOutput* output)
		{
			PR_ASSERT(output);
			PR_ASSERT(!hasScalarShaderOutput(name));
			mNamedScalarShaderOutputs[name] = output;
			mScalarShaderOutputs.push_back(output);
		}

		inline void addShaderOutput(PR::ScalarShaderOutput* output)
		{
			PR_ASSERT(output);
			mScalarShaderOutputs.push_back(output);
		}

		inline PR::ScalarShaderOutput* getScalarShaderOutput(const std::string& name) const
		{
			return mNamedScalarShaderOutputs.at(name);
		}

		inline bool hasScalarShaderOutput(const std::string& name) const
		{
			return mNamedScalarShaderOutputs.count(name) != 0;
		}

		inline void addShaderOutput(const std::string& name, PR::SpectralShaderOutput* output)
		{
			PR_ASSERT(output);
			PR_ASSERT(!hasSpectralShaderOutput(name));
			mNamedSpectralShaderOutputs[name] = output;
			mSpectralShaderOutputs.push_back(output);
		}

		inline void addShaderOutput(PR::SpectralShaderOutput* output)
		{
			PR_ASSERT(output);
			mSpectralShaderOutputs.push_back(output);
		}

		inline PR::SpectralShaderOutput* getSpectralShaderOutput(const std::string& name) const
		{
			return mNamedSpectralShaderOutputs.at(name);
		}

		inline bool hasSpectralShaderOutput(const std::string& name) const
		{
			return mNamedSpectralShaderOutputs.count(name) != 0;
		}

		inline void addShaderOutput(const std::string& name, PR::VectorShaderOutput* output)
		{
			PR_ASSERT(output);
			PR_ASSERT(!hasVectorShaderOutput(name));
			mNamedVectorShaderOutputs[name] = output;
			mVectorShaderOutputs.push_back(output);
		}

		inline void addShaderOutput(PR::VectorShaderOutput* output)
		{
			PR_ASSERT(output);
			mVectorShaderOutputs.push_back(output);
		}

		inline PR::VectorShaderOutput* getVectorShaderOutput(const std::string& name) const
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

		inline void setCamera(PR::Camera* cam)
		{
			mCamera = cam;
		}

		inline PR::uint32 renderWidth() const
		{
			return mRenderWidth;
		}

		inline void setRenderWidth(PR::uint32 i)
		{
			mRenderWidth = i;
		}

		inline PR::uint32 renderHeight() const
		{
			return mRenderHeight;
		}

		inline void setRenderHeight(PR::uint32 i)
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
	private:
		PR::Scene mScene;
		PR::Camera* mCamera;
		PR::uint32 mRenderWidth;
		PR::uint32 mRenderHeight;

		float mCropMinX;
		float mCropMaxX;
		float mCropMinY;
		float mCropMaxY;

		std::map<std::string, PR::Spectrum> mSpectrums;
		std::map<std::string, PR::Material*> mMaterials;

		std::map<std::string, PR::TriMesh*> mMeshes;

		std::list<PR::ScalarShaderOutput*> mScalarShaderOutputs;
		std::list<PR::SpectralShaderOutput*> mSpectralShaderOutputs;
		std::list<PR::VectorShaderOutput*> mVectorShaderOutputs;

		std::map<std::string, PR::ScalarShaderOutput*> mNamedScalarShaderOutputs;
		std::map<std::string, PR::SpectralShaderOutput*> mNamedSpectralShaderOutputs;
		std::map<std::string, PR::VectorShaderOutput*> mNamedVectorShaderOutputs;

		OIIO::TextureSystem* mTextureSystem;
		OutputSpecification mOutputSpecification;
	};
}
