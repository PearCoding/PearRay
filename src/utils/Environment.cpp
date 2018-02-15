#include "Environment.h"

#include "entity/Entity.h"
#include "geometry/TriMesh.h"
#include "material/Material.h"

#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include "Logger.h"

namespace PR {
Environment::Environment(const std::shared_ptr<SpectrumDescriptor>& desc, const std::string& name)
	: mScene(name)
	, mRenderWidth(1920)
	, mRenderHeight(1080)
	, mCropMinX(0)
	, mCropMaxX(1)
	, mCropMinY(0)
	, mCropMaxY(1)
	, mSpectrumDescriptor(desc)
{
	//Defaults
	auto addColor = [&](const std::string& name, float r, float g, float b) {
		Spectrum c(desc);
		RGBConverter::toSpec(c, r, g, b);
		mSpectrums.insert(std::make_pair(name, c));
	};

	addColor("black", 0, 0, 0);
	addColor("white", 1, 1, 1);
	addColor("red", 1, 0, 0);
	addColor("green", 0, 1, 0);
	addColor("blue", 0, 0, 1);
	addColor("magenta", 1, 0, 1);
	addColor("yellow", 1, 1, 0);
	addColor("cyan", 0, 1, 1);
	addColor("gray", 0.5f, 0.5f, 0.5f);
	addColor("lightGray", 0.666f, 0.666f, 0.666f);
	addColor("darkGray", 0.333f, 0.333f, 0.333f);

	mTextureSystem = OIIO::TextureSystem::create();
}

Environment::~Environment()
{
	mScene.clear();
	OIIO::TextureSystem::destroy(mTextureSystem);
}

void Environment::dumpInformation() const
{
	for (const auto& p : mScene.entities())
		PR_LOGGER.logf(L_Info, M_Entity, "%s:\n%s",
					   p->name().c_str(), p->dumpInformation().c_str());

	for (const auto& p : mMaterials)
		PR_LOGGER.logf(L_Info, M_Material, "%s:\n%s",
					   p.first.c_str(), p.second->dumpInformation().c_str());
}

void Environment::setup(const std::shared_ptr<RenderContext>& renderer)
{
	PR_LOGGER.log(L_Info, M_Scene, "Freezing scene");
	mScene.freeze();
	PR_LOGGER.log(L_Info, M_Scene, "Starting to build space-partitioning structure");
	mScene.buildTree();
	PR_LOGGER.log(L_Info, M_Scene, "Initializing output");
	mOutputSpecification.setup(renderer);
	PR_LOGGER.log(L_Info, M_Scene, "Initializing scene");
	mScene.setup(renderer);
}

void Environment::save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force) const
{
	mOutputSpecification.save(renderer, toneMapper, force);
}
} // namespace PR
