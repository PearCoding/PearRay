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

		mTextureSystem = OIIO::TextureSystem::create();
	}

	Environment::~Environment()
	{
		mScene.clear();
		for (std::pair<std::string, PR::Material*> p : mMaterials)
		{
			delete p.second;
		}

		for (std::pair<std::string, PR::TriMesh*> p : mMeshes)
		{
			delete p.second;
		}

		// TODO: Destructor virtual?
		for (PR::ScalarShaderOutput* s : mScalarShaderOutputs)
		{
			delete s;
		}

		for (PR::SpectralShaderOutput* s : mSpectralShaderOutputs)
		{
			delete s;
		}

		for (PR::VectorShaderOutput* s : mVectorShaderOutputs)
		{
			delete s;
		}

		OIIO::TextureSystem::destroy(mTextureSystem);
	}
	
	void Environment::dumpInformation() const
	{
		using namespace PR;
		for (PR::Entity* p : mScene.entities())
			PR_LOGGER.logf(L_Info, M_Entity, "%s:\n%s",
				p->name().c_str(), p->dumpInformation().c_str());

		for (std::pair<std::string, PR::Material*> p : mMaterials)
			PR_LOGGER.logf(L_Info, M_Material, "%s:\n%s",
				p.first.c_str(), p.second->dumpInformation().c_str());
	}
}