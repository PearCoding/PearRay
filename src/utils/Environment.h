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
	class IMesh;
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

		inline PR::IMesh* getMesh(const std::string& name) const
		{
			return mMeshes.at(name);
		}

		inline bool hasMesh(const std::string& name) const
		{
			return mMeshes.count(name) != 0;
		}

		inline void addMesh(const std::string& name, PR::IMesh* m)
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

		std::map<std::string, PR::IMesh*> mMeshes;

		std::map<std::string, PR::Texture1D*> mFileTexture1D;
		std::map<std::string, PR::Texture2D*> mFileTexture2D;

		std::list<PR::Data1D*> mData1D;
		std::list<PR::Data2D*> mData2D;
		std::list<PR::Texture1D*> mTexture1D;
		std::list<PR::Texture2D*> mTexture2D;
	};
}
