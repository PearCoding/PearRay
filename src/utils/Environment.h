#pragma once

#include "scene/Scene.h"
#include "spectral/Spectrum.h"

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
		Environment(const std::string& name);
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
	};
}
