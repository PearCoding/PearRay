#include "Environment.h"

#include "entity/VirtualEntity.h"
#include "geometry/TriMesh.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "renderer/RenderFactory.h"

#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include "Logger.h"

namespace PR {
Environment::Environment(bool useStandardLib)
	: mRenderManager()
{
	mTextureSystem = OIIO::TextureSystem::create();

	if (useStandardLib) {
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

		// Load
		mRenderManager.materialManager()->loadFactory(mRenderManager.registry(),
			"", "lambert");
	}
}

Environment::~Environment()
{
	mSceneFactory.clear();
	OIIO::TextureSystem::destroy(mTextureSystem);
}

uint32 Environment::renderWidth() const
{
	return RenderSettings(mRenderManager.registry()).filmWidth();
}

void Environment::setRenderWidth(uint32 i)
{
	mRegistry->setByGroup(RG_RENDERER, "film/width", i);
}

uint32 Environment::renderHeight() const
{
	return RenderSettings(mRenderManager.registry()).filmHeight();
}

void Environment::setRenderHeight(uint32 i)
{
	mRenderManager.registry()->setByGroup(RG_RENDERER, "film/height", i);
}

void Environment::setCrop(float xmin, float xmax, float ymin, float ymax)
{
	mRenderManager.registry()->setByGroup(RG_RENDERER, "film/crop/min_x", xmin);
	mRenderManager.registry()->setByGroup(RG_RENDERER, "film/crop/max_x", xmax);
	mRenderManager.registry()->setByGroup(RG_RENDERER, "film/crop/min_y", ymin);
	mRenderManager.registry()->setByGroup(RG_RENDERER, "film/crop/max_y", ymax);
}

float Environment::cropMinX() const
{
	return RenderSettings(mRenderManager.registry()).cropMinX();
}

float Environment::cropMaxX() const
{
	return RenderSettings(mRenderManager.registry()).cropMaxX();
}

float Environment::cropMinY() const
{
	return RenderSettings(mRenderManager.registry()).cropMinY();
}

float Environment::cropMaxY() const
{
	return RenderSettings(mRenderManager.registry()).cropMaxY();
}

void Environment::dumpInformation() const
{
	for (auto p : mSceneFactory.entities())
		PR_LOG(L_INFO) << p->name() << ":" << std::endl
					   << p->dumpInformation() << std::endl;

	for (auto p : mMaterials)
		PR_LOG(L_INFO) << p.first << ":" << std::endl
					   << p.second->dumpInformation() << std::endl;
}

void Environment::setup(const std::shared_ptr<RenderContext>& renderer)
{
	PR_LOG(L_INFO) << "Initializing output" << std::endl;
	mOutputSpecification.setup(renderer);
}

void Environment::save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force) const
{
	mOutputSpecification.save(renderer, toneMapper, force);
}
} // namespace PR
