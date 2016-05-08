#pragma once

#include "scene/Scene.h"
#include "spectral/Spectrum.h"

#include "texture/Texture1D.h"
#include "texture/Texture2D.h"

#include <map>

namespace PR
{
	class Camera;
	class Material;
	class Mesh;
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

		inline PR::Mesh* getMesh(const std::string& name) const
		{
			return mMeshes.at(name);
		}

		inline bool hasMesh(const std::string& name) const
		{
			return mMeshes.count(name) != 0;
		}

		inline void addMesh(const std::string& name, PR::Mesh* m)
		{
			PR_ASSERT(m && !hasMesh(name));
			mMeshes[name] = m;
		}

		inline PR::Texture1D* getTexture1D(const std::string& filename) const
		{
			return mFileTexture1D.at(filename);
		}

		inline bool hasTexture1D(const std::string& filename) const
		{
			return mFileTexture1D.count(filename) != 0;
		}

		inline void addTexture1D(const std::string& filename, PR::Texture1D* tex)
		{
			PR_ASSERT(tex && !hasTexture1D(filename));
			mFileTexture1D[filename] = tex;
			mTexture1D.push_back(tex);
		}

		inline void addTexture1D(PR::Texture1D* tex)
		{
			PR_ASSERT(tex);
			mTexture1D.push_back(tex);
		}

		inline PR::Texture2D* getTexture2D(const std::string& filename) const
		{
			return mFileTexture2D.at(filename);
		}

		inline bool hasTexture2D(const std::string& filename) const
		{
			return mFileTexture2D.count(filename) != 0;
		}

		inline void addTexture2D(const std::string& filename, PR::Texture2D* tex)
		{
			PR_ASSERT(tex && !hasTexture2D(filename));
			mFileTexture2D[filename] = tex;
			mTexture2D.push_back(tex);
		}

		inline void addTexture2D(PR::Texture2D* tex)
		{
			PR_ASSERT(tex);
			mTexture2D.push_back(tex);
		}

		inline void addData1D(PR::Data1D* tex)
		{
			PR_ASSERT(tex);
			mData1D.push_back(tex);
		}

		inline void addData2D(PR::Data2D* tex)
		{
			PR_ASSERT(tex);
			mData2D.push_back(tex);
		}

		inline PR::Camera* camera() const
		{
			return mCamera;
		}

		inline void setCamera(PR::Camera* cam)
		{
			mCamera = cam;
		}
	private:
		PR::Scene mScene;
		PR::Camera* mCamera;
		std::map<std::string, PR::Spectrum> mSpectrums;
		std::map<std::string, PR::Material*> mMaterials;
		std::map<std::string, PR::Mesh*> mMeshes;

		std::map<std::string, PR::Texture1D*> mFileTexture1D;
		std::map<std::string, PR::Texture2D*> mFileTexture2D;

		std::list<PR::Data1D*> mData1D;
		std::list<PR::Data2D*> mData2D;
		std::list<PR::Texture1D*> mTexture1D;
		std::list<PR::Texture2D*> mTexture2D;
	};
}
