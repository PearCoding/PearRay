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
		mSpectrums["black"] = PR::Spectrum();
		mSpectrums["white"] = PR::RGBConverter::White;
		mSpectrums["red"] = PR::RGBConverter::Red;
		mSpectrums["green"] = PR::RGBConverter::Green;
		mSpectrums["blue"] = PR::RGBConverter::Blue;
		mSpectrums["magenta"] = PR::RGBConverter::Magenta;
		mSpectrums["yellow"] = PR::RGBConverter::Yellow;
		mSpectrums["cyan"] = PR::RGBConverter::Cyan;
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