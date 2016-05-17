#pragma once

#include "Config.h"
#include "PearMath.h"

#include "texture/Texture1D.h"
#include "texture/Texture2D.h"

#include "loader/ImageLoader.h"

#include <map>
#include <string>

namespace DL
{
	class Data;
	class DataArray;
	class DataGroup;
}

namespace PR
{
	class Entity;
	class IMesh;
}

namespace PRU
{
	class Environment;
	class PR_LIB_UTILS SceneLoader
	{
	public:
		SceneLoader();
		virtual ~SceneLoader();

		Environment* loadFromFile(const std::string& path);
		Environment* load(const std::string& source);

		PM::vec3 getVector(DL::DataArray* arr, bool& ok) const;
		PM::quat getRotation(DL::Data* data, bool& ok) const;

		PR::Texture2D* getTexture2D(Environment* env, DL::Data* data) const;
		PR::Data1D* getData1D(Environment* env, DL::Data* data) const;
		PR::Data2D* getData2D(Environment* env, DL::Data* data) const;

	private:
		void addEntity(DL::DataGroup* group, PR::Entity* parent, Environment* env);
		void addMaterial(DL::DataGroup* group, Environment* env);
		void addSpectrum(DL::DataGroup* group, Environment* env);
		void addSubGraph(DL::DataGroup* group, Environment* env);

		ImageLoader mImageLoader;
	};
}
