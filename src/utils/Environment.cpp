#include "Environment.h"

#include "geometry/IMesh.h"
#include "material/Material.h"

#include "spectral/XYZConverter.h"
#include "spectral/RGBConverter.h"

namespace PRU
{
	Environment::Environment(const std::string& name) :
		mScene(name), mCamera(nullptr), mRenderWidth(1920), mRenderHeight(1080),
		mCropMinX(0), mCropMaxX(1), mCropMinY(0), mCropMaxY(1), mBackgroundMaterial(nullptr)
	{
		PR::XYZConverter::init();
		PR::RGBConverter::init();
	}

	Environment::~Environment()
	{
		mScene.clear();
		for (std::pair<std::string, PR::Material*> p : mMaterials)
		{
			delete p.second;
		}

		for (std::pair<std::string, PR::IMesh*> p : mMeshes)
		{
			delete p.second;
		}

		for (PR::Texture1D* tex : mTexture1D)
		{
			delete tex;
		}

		for (PR::Texture2D* tex : mTexture2D)
		{
			delete tex;
		}

		for (PR::Data1D* tex : mData1D)
		{
			delete tex;
		}

		for (PR::Data2D* tex : mData2D)
		{
			delete tex;
		}
	}
}