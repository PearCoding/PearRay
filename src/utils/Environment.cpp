#include "Environment.h"

#include "geometry/TriMesh.h"
#include "entity/Entity.h"
#include "material/Material.h"

#include "spectral/XYZConverter.h"
#include "spectral/RGBConverter.h"

#include "Logger.h"

namespace PRU
{
	Environment::Environment(const std::string& name) :
		mScene(name), mCamera(nullptr), mRenderWidth(1920), mRenderHeight(1080),
		mCropMinX(0), mCropMaxX(1), mCropMinY(0), mCropMaxY(1)
	{
		//Defaults
		mSpectrums["black"] = PR::Spectrum();
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

		mTextureSystem = OIIO::TextureSystem::create();
	}

	Environment::~Environment()
	{
		mScene.clear();
		OIIO::TextureSystem::destroy(mTextureSystem);
	}
	
	void Environment::dumpInformation() const
	{
		using namespace PR;
		for (const auto& p : mScene.entities())
			PR_LOGGER.logf(L_Info, M_Entity, "%s:\n%s",
				p->name().c_str(), p->dumpInformation().c_str());

		for (const auto& p : mMaterials)
			PR_LOGGER.logf(L_Info, M_Material, "%s:\n%s",
				p.first.c_str(), p.second->dumpInformation().c_str());
	}
}