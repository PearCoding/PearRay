#include "Environment.h"

#include "geometry/IMesh.h"
#include "material/Material.h"

#include "spectral/XYZConverter.h"
#include "spectral/RGBConverter.h"

namespace PRU
{
	Environment::Environment(const std::string& name) :
		mScene(name), mCamera(nullptr), mRenderWidth(1920), mRenderHeight(1080),
		mCropMinX(0), mCropMaxX(0), mCropMinY(0), mCropMaxY(0), mBackgroundMaterial(nullptr)
	{
		PR::XYZConverter::init();
		PR::RGBConverter::init();

		//Defaults
		mSpectrums["black"] = PR::RGBConverter::toSpec(0, 0, 0);
		mSpectrums["white"] = PR::RGBConverter::toSpec(1, 1, 1);
		mSpectrums["red"] = PR::RGBConverter::toSpec(1, 0, 0);
		mSpectrums["green"] = PR::RGBConverter::toSpec(0, 1, 0);
		mSpectrums["blue"] = PR::RGBConverter::toSpec(0, 0, 1);
		mSpectrums["magenta"] = PR::RGBConverter::toSpec(1, 0, 1);
		mSpectrums["yellow"] = PR::RGBConverter::toSpec(1, 1, 0);
		mSpectrums["cyan"] = PR::RGBConverter::toSpec(0, 1, 1);
		mSpectrums["gray"] = PR::RGBConverter::toSpec(0.5f, 0.5f, 0.5f);
		mSpectrums["lightGray"] = PR::RGBConverter::toSpec(0.666f, 0.666f, 0.666f);
		mSpectrums["darkGray"] = PR::RGBConverter::toSpec(0.333f, 0.333f, 0.333f);

		mSpectrums["sun_norm"] = PR::Spectrum::fromBlackbodyNorm(5500);
		mSpectrums["candle_norm"] = PR::Spectrum::fromBlackbodyNorm(1000);
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