#include "SystemPropertyView.h"
#include "PropertyView.h"

#include "properties/PropertyTable.h"

#include "properties/GroupProperty.h"
#include "properties/IntProperty.h"
#include "properties/DoubleProperty.h"
#include "properties/ButtonProperty.h"
#include "properties/BoolProperty.h"
#include "properties/SelectionProperty.h"

#include "renderer/Renderer.h"
#include "renderer/RenderSettings.h"

#include <QBoxLayout>

SystemPropertyView::SystemPropertyView(QWidget* parent) :
QWidget(parent)
{
	mView = new PropertyView(this);
	
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(mView);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	// Setup properties
	mRendererGroupProp = new GroupProperty(tr("Renderer"));

	mRendererTileXProp = new IntProperty(tr("Tile X Count"), 5, 1, 128);
	mRendererGroupProp->addChild(mRendererTileXProp);

	mRendererTileYProp = new IntProperty(tr("Tile Y Count"), 5, 1, 128);
	mRendererGroupProp->addChild(mRendererTileYProp);

	mRendererThreadsProp = new IntProperty(tr("Threads"), 0, -10000, 10000);
	mRendererThreadsProp->setToolTip(tr("0 = Automatic"));
	mRendererGroupProp->addChild(mRendererThreadsProp);

	mRendererIncremental = new BoolProperty(tr("Incremental"), true);
	mRendererGroupProp->addChild(mRendererIncremental);

	mRendererMaxDiffuseBouncesProp = new IntProperty(tr("Max Diffuse Bounces"), 4, 0, 4096);
	mRendererGroupProp->addChild(mRendererMaxDiffuseBouncesProp);

	mRendererMaxRayDepthProp = new IntProperty(tr("Max Ray Depth"), 10, 1, 4096);
	mRendererGroupProp->addChild(mRendererMaxRayDepthProp);

	mDebugVisualizationProp = new SelectionProperty(tr("Debug"), PR::DM_None);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("None"), PR::DM_None);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Depth"), PR::DM_Depth);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Normal Both"), PR::DM_Normal_Both);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Normal Positive"), PR::DM_Normal_Positive);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Normal Negative"), PR::DM_Normal_Negative);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Normal Spherical"), PR::DM_Normal_Spherical);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Tangent Both"), PR::DM_Tangent_Both);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Tangent Positive"), PR::DM_Tangent_Positive);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Tangent Negative"), PR::DM_Tangent_Negative);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Tangent Spherical"), PR::DM_Tangent_Spherical);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Binormal Both"), PR::DM_Binormal_Both);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Binormal Positive"), PR::DM_Binormal_Positive);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Binormal Negative"), PR::DM_Binormal_Negative);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Binormal Spherical"), PR::DM_Binormal_Spherical);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("UV"), PR::DM_UV);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("PDF"), PR::DM_PDF);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Applied"), PR::DM_Applied);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Validity"), PR::DM_Validity);
	mRendererGroupProp->addChild(mDebugVisualizationProp);

	mProperties.add(mRendererGroupProp);

	// Sampler
	mPixelSamplerGroupProp = new GroupProperty(tr("Pixel Sampler"));

	mPixelSamplerProp = new SelectionProperty(tr("Mode"), PR::SM_MultiJitter);
	((SelectionProperty*)mPixelSamplerProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mPixelSamplerProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mPixelSamplerProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mPixelSamplerProp)->addItem(tr("Multi Jitter"), PR::SM_MultiJitter);
	mPixelSamplerGroupProp->addChild(mPixelSamplerProp);

	mPixelSamplesProp = new IntProperty(tr("Max Samples"), 8, 1, 1024);
	mPixelSamplerGroupProp->addChild(mPixelSamplesProp);
	mProperties.add(mPixelSamplerGroupProp);

	// Direct Lightning
	mDirectLightningGroupProp = new GroupProperty(tr("Direct Lightning"));
	mMaxLightSamplesProp = new IntProperty(tr("Max Light Samples"), 2);
	mDirectLightningGroupProp->addChild(mMaxLightSamplesProp);
	mUseBiDirectProp = new BoolProperty(tr("Use BiDirect"), true);
	mDirectLightningGroupProp->addChild(mUseBiDirectProp);
	mProperties.add(mDirectLightningGroupProp);

	// Photon Mapping
	mPhotonMappingGroupProp = new GroupProperty(tr("Photon Mapping"));
	mMaxPhotonsProp = new IntProperty(tr("Max Photons"), 1000, 0, 10000000);
	mPhotonMappingGroupProp->addChild(mMaxPhotonsProp);
	mMaxPhotonGatherCountProp = new IntProperty(tr("Max Gather Count"), 500, 0);
	mPhotonMappingGroupProp->addChild(mMaxPhotonGatherCountProp);
	mMaxPhotonGatherRadiusProp = new DoubleProperty(tr("Max Gather Radius"), 0.1, 0, 10000);
	mPhotonMappingGroupProp->addChild(mMaxPhotonGatherRadiusProp);
	mMaxPhotonDiffuseBouncesProp = new IntProperty(tr("Max Diffuse Bounces"), 4, 0);
	mPhotonMappingGroupProp->addChild(mMaxPhotonDiffuseBouncesProp);
	mMinPhotonSpecularBouncesProp = new IntProperty(tr("Min Specular Bounces"), 1, 0);
	mPhotonMappingGroupProp->addChild(mMinPhotonSpecularBouncesProp);
	mPhotonGatheringModeProp = new SelectionProperty(tr("Gathering Mode"), PR::PGM_Sphere);
	((SelectionProperty*)mPhotonGatheringModeProp)->addItem(tr("Sphere"), PR::PGM_Sphere);
	((SelectionProperty*)mPhotonGatheringModeProp)->addItem(tr("Dome"), PR::PGM_Dome);
	mPhotonMappingGroupProp->addChild(mPhotonGatheringModeProp);
	mPhotonSqueezeWeightProp = new DoubleProperty(tr("Squeeze Weight"), 0, 0, 1);
	mPhotonMappingGroupProp->addChild(mPhotonSqueezeWeightProp);
	mProperties.add(mPhotonMappingGroupProp);

	mView->setPropertyTable(&mProperties);
	mView->expandToDepth(1);

	connect(&mProperties, SIGNAL(valueChanged(IProperty*)), this, SLOT(propertyValueChanged(IProperty*)));
}

SystemPropertyView::~SystemPropertyView()
{
	delete mRendererGroupProp;
	delete mRendererTileXProp;
	delete mRendererTileYProp;
	delete mRendererThreadsProp;
	delete mRendererIncremental;
	delete mPixelSamplesProp;
	delete mPixelSamplerProp;
	delete mRendererMaxDiffuseBouncesProp;
	delete mRendererMaxRayDepthProp;
	delete mDebugVisualizationProp;
	delete mMaxLightSamplesProp;
	delete mUseBiDirectProp;
	delete mMaxPhotonsProp;
	delete mMaxPhotonGatherCountProp;
	delete mMaxPhotonGatherRadiusProp;
	delete mMaxPhotonDiffuseBouncesProp;
	delete mMinPhotonSpecularBouncesProp;
	delete mPhotonGatheringModeProp;
	delete mPhotonSqueezeWeightProp;
	delete mPhotonMappingGroupProp;
}

int SystemPropertyView::getTileX() const
{
	return reinterpret_cast<IntProperty*>(mRendererTileXProp)->value();
}

int SystemPropertyView::getTileY() const
{
	return reinterpret_cast<IntProperty*>(mRendererTileYProp)->value();
}

int SystemPropertyView::getThreadCount() const
{
	return reinterpret_cast<IntProperty*>(mRendererThreadsProp)->value();
}

void SystemPropertyView::propertyValueChanged(IProperty* prop)
{
}

void SystemPropertyView::fillContent(PR::Renderer* renderer)
{
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setValue(0);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setDefaultValue(0);

	reinterpret_cast<BoolProperty*>(mRendererIncremental)->setValue(renderer->settings().isIncremental());
	reinterpret_cast<BoolProperty*>(mRendererIncremental)->setDefaultValue(renderer->settings().isIncremental());

	reinterpret_cast<IntProperty*>(mPixelSamplesProp)->setValue(renderer->settings().maxPixelSampleCount());
	reinterpret_cast<IntProperty*>(mPixelSamplesProp)->setDefaultValue(renderer->settings().maxPixelSampleCount());
	reinterpret_cast<SelectionProperty*>(mPixelSamplerProp)->setIndex(renderer->settings().pixelSampler());
	reinterpret_cast<SelectionProperty*>(mPixelSamplerProp)->setDefaultIndex(renderer->settings().pixelSampler());

	reinterpret_cast<IntProperty*>(mRendererMaxDiffuseBouncesProp)->setValue(renderer->settings().maxDiffuseBounces());
	reinterpret_cast<IntProperty*>(mRendererMaxDiffuseBouncesProp)->setDefaultValue(renderer->settings().maxDiffuseBounces());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setValue(renderer->settings().maxRayDepth());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setDefaultValue(renderer->settings().maxRayDepth());
	reinterpret_cast<SelectionProperty*>(mDebugVisualizationProp)->setIndex(renderer->settings().debugMode());
	reinterpret_cast<SelectionProperty*>(mDebugVisualizationProp)->setDefaultIndex(renderer->settings().debugMode());

	reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->setValue(renderer->settings().maxLightSamples());
	reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->setDefaultValue(renderer->settings().maxLightSamples());
	reinterpret_cast<BoolProperty*>(mUseBiDirectProp)->setValue(renderer->settings().isBiDirect());
	reinterpret_cast<BoolProperty*>(mUseBiDirectProp)->setDefaultValue(renderer->settings().isBiDirect());

	reinterpret_cast<IntProperty*>(mMaxPhotonsProp)->setValue(renderer->settings().maxPhotons());
	reinterpret_cast<IntProperty*>(mMaxPhotonsProp)->setDefaultValue(renderer->settings().maxPhotons());
	reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->setValue(
		renderer->settings().maxPhotonGatherRadius());
	reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->setDefaultValue(
		renderer->settings().maxPhotonGatherRadius());
	reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->setValue(renderer->settings().maxPhotonGatherCount());
	reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->setDefaultValue(renderer->settings().maxPhotonGatherCount());
	reinterpret_cast<IntProperty*>(mMaxPhotonDiffuseBouncesProp)->setValue(renderer->settings().maxPhotonDiffuseBounces());
	reinterpret_cast<IntProperty*>(mMaxPhotonDiffuseBouncesProp)->setDefaultValue(renderer->settings().maxPhotonDiffuseBounces());
	reinterpret_cast<IntProperty*>(mMinPhotonSpecularBouncesProp)->setValue(renderer->settings().minPhotonSpecularBounces());
	reinterpret_cast<IntProperty*>(mMinPhotonSpecularBouncesProp)->setDefaultValue(renderer->settings().minPhotonSpecularBounces());
	reinterpret_cast<SelectionProperty*>(mPhotonGatheringModeProp)->setIndex(renderer->settings().photonGatheringMode());
	reinterpret_cast<SelectionProperty*>(mPhotonGatheringModeProp)->setDefaultIndex(renderer->settings().photonGatheringMode());
	reinterpret_cast<DoubleProperty*>(mPhotonSqueezeWeightProp)->setValue(renderer->settings().photonSqueezeWeight());
	reinterpret_cast<DoubleProperty*>(mPhotonSqueezeWeightProp)->setDefaultValue(renderer->settings().photonSqueezeWeight());
}

void SystemPropertyView::setupRenderer(PR::Renderer* renderer)
{
	renderer->settings().setMaxDiffuseBounces(reinterpret_cast<IntProperty*>(mRendererMaxDiffuseBouncesProp)->value());
	renderer->settings().setMaxRayDepth(reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->value());
	renderer->settings().setDebugMode((PR::DebugMode)reinterpret_cast<SelectionProperty*>(mDebugVisualizationProp)->index());
	renderer->settings().setIncremental(reinterpret_cast<BoolProperty*>(mRendererIncremental)->value());

	renderer->settings().setPixelSampler((PR::SamplerMode)reinterpret_cast<SelectionProperty*>(mPixelSamplerProp)->index());
	renderer->settings().setMaxPixelSampleCount(reinterpret_cast<IntProperty*>(mPixelSamplesProp)->value());

	renderer->settings().setMaxLightSamples(reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->value());
	renderer->settings().enableBiDirect(reinterpret_cast<BoolProperty*>(mUseBiDirectProp)->value());

	renderer->settings().setMaxPhotons(reinterpret_cast<IntProperty*>(mMaxPhotonsProp)->value());
	renderer->settings().setMaxPhotonGatherCount(reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->value());
	renderer->settings().setMaxPhotonGatherRadius(reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->value());
	renderer->settings().setMaxPhotonDiffuseBounces(reinterpret_cast<IntProperty*>(mMaxPhotonDiffuseBouncesProp)->value());
	renderer->settings().setMinPhotonSpecularBounces(reinterpret_cast<IntProperty*>(mMinPhotonSpecularBouncesProp)->value());
	renderer->settings().setPhotonGatheringMode((PR::PhotonGatheringMode)reinterpret_cast<SelectionProperty*>(mPhotonGatheringModeProp)->index());
	renderer->settings().setPhotonSqueezeWeight(reinterpret_cast<DoubleProperty*>(mPhotonSqueezeWeightProp)->value());
}

