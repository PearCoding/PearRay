#include "SystemPropertyView.h"
#include "PropertyView.h"

#include "properties/PropertyTable.h"

#include "properties/GroupProperty.h"
#include "properties/IntProperty.h"
#include "properties/DoubleProperty.h"
#include "properties/ButtonProperty.h"
#include "properties/BoolProperty.h"
#include "properties/SelectionProperty.h"

#include "renderer/RenderFactory.h"
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
	mRendererGroupProp = new GroupProperty(tr("RenderContext"));

	mRendererTileXProp = new IntProperty(tr("Tile X Count"), 5, 1, 128);
	mRendererGroupProp->addChild(mRendererTileXProp);

	mRendererTileYProp = new IntProperty(tr("Tile Y Count"), 5, 1, 128);
	mRendererGroupProp->addChild(mRendererTileYProp);

	mRendererTileModeProp = new SelectionProperty(tr("Tile Mode"), PR::TM_Linear);
	((SelectionProperty*)mRendererTileModeProp)->addItem(tr("Linear"), PR::TM_Linear);
	((SelectionProperty*)mRendererTileModeProp)->addItem(tr("Tile"), PR::TM_Tile);
	((SelectionProperty*)mRendererTileModeProp)->addItem(tr("Spiral"), PR::TM_Spiral);
	mRendererGroupProp->addChild(mRendererTileModeProp);

	mRendererThreadsProp = new IntProperty(tr("Threads"), 0, -10000, 10000);
	mRendererThreadsProp->setToolTip(tr("0 = Automatic"));
	mRendererGroupProp->addChild(mRendererThreadsProp);

	mRendererIncremental = new BoolProperty(tr("Incremental"), true);
	mRendererGroupProp->addChild(mRendererIncremental);

	mRendererMaxDiffuseBouncesProp = new IntProperty(tr("Max Diffuse Bounces"), 4, 0, 4096);
	mRendererGroupProp->addChild(mRendererMaxDiffuseBouncesProp);

	mRendererMaxRayDepthProp = new IntProperty(tr("Max Ray Depth"), 10, 1, 4096);
	mRendererGroupProp->addChild(mRendererMaxRayDepthProp);

	mIntegratorProp = new SelectionProperty(tr("Integrator"), PR::IM_BiDirect);
	((SelectionProperty*)mIntegratorProp)->addItem(tr("Direct"), PR::IM_Direct);
	((SelectionProperty*)mIntegratorProp)->addItem(tr("BiDirect"), PR::IM_BiDirect);
	((SelectionProperty*)mIntegratorProp)->addItem(tr("Progressive Photon Mapping"), PR::IM_PPM);
	mRendererGroupProp->addChild(mIntegratorProp);

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
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("UVW"), PR::DM_UVW);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("PDF"), PR::DM_PDF);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Emission"), PR::DM_Emission);
	((SelectionProperty*)mDebugVisualizationProp)->addItem(tr("Validity"), PR::DM_Validity);
	mRendererGroupProp->addChild(mDebugVisualizationProp);

	mProperties.add(mRendererGroupProp);

	// Sampler
	mSamplerGroupProp = new GroupProperty(tr("Sampler"));

	mSamplerAAProp = new SelectionProperty(tr("AA Mode"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerAAProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mSamplerAAProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mSamplerAAProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mSamplerAAProp)->addItem(tr("Multi Jitter"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerAAProp)->addItem(tr("Halton QMC"), PR::SM_HaltonQMC);
	mSamplerGroupProp->addChild(mSamplerAAProp);

	mSamplerAAMaxProp = new IntProperty(tr("Max AA Samples"), 8, 1, 4096);
	mSamplerGroupProp->addChild(mSamplerAAMaxProp);

	mSamplerLensProp = new SelectionProperty(tr("Lens Mode"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerLensProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mSamplerLensProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mSamplerLensProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mSamplerLensProp)->addItem(tr("Multi Jitter"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerLensProp)->addItem(tr("Halton QMC"), PR::SM_HaltonQMC);
	mSamplerGroupProp->addChild(mSamplerLensProp);
	
	mSamplerLensMaxProp = new IntProperty(tr("Max Lens Samples"), 1, 1, 4096);
	mSamplerGroupProp->addChild(mSamplerLensMaxProp);
	
	mSamplerTimeProp = new SelectionProperty(tr("Time Mode"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerTimeProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mSamplerTimeProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mSamplerTimeProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mSamplerTimeProp)->addItem(tr("Multi Jitter"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerTimeProp)->addItem(tr("Halton QMC"), PR::SM_HaltonQMC);
	mSamplerGroupProp->addChild(mSamplerTimeProp);
	
	mSamplerTimeMaxProp = new IntProperty(tr("Max Time Samples"), 1, 1, 4096);
	mSamplerGroupProp->addChild(mSamplerTimeMaxProp);
	
	mSamplerTimeMappingProp = new SelectionProperty(tr("Time Mapping Mode"), PR::TMM_Center);
	((SelectionProperty*)mSamplerTimeMappingProp)->addItem(tr("Center [-1/2,1/2]"), PR::TMM_Center);
	((SelectionProperty*)mSamplerTimeMappingProp)->addItem(tr("Left [-1,0]"), PR::TMM_Left);
	((SelectionProperty*)mSamplerTimeMappingProp)->addItem(tr("Right [0,1]"), PR::TMM_Right);
	mSamplerGroupProp->addChild(mSamplerTimeMappingProp);
	
	mSamplerTimeScaleProp = new DoubleProperty(tr("Time Scale"), 1, 0, 1000);
	mSamplerGroupProp->addChild(mSamplerTimeScaleProp);
	
	mSamplerSpectralProp = new SelectionProperty(tr("Spectral Mode"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerSpectralProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mSamplerSpectralProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mSamplerSpectralProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mSamplerSpectralProp)->addItem(tr("Multi Jitter"), PR::SM_MultiJitter);
	((SelectionProperty*)mSamplerSpectralProp)->addItem(tr("Halton QMC"), PR::SM_HaltonQMC);
	mSamplerGroupProp->addChild(mSamplerSpectralProp);
	
	mSamplerSpectralMaxProp = new IntProperty(tr("Max Spectral Samples"), 1, 1, 4096);
	mSamplerGroupProp->addChild(mSamplerSpectralMaxProp);
	
	mProperties.add(mSamplerGroupProp);

	// Direct Lightning
	mDirectLightningGroupProp = new GroupProperty(tr("Direct Lightning"));
	mMaxLightSamplesProp = new IntProperty(tr("Max Light Samples"), 2);
	mDirectLightningGroupProp->addChild(mMaxLightSamplesProp);

	// PPM
	mPPMGroupProp = new GroupProperty(tr("Progressive Photon Mapping (PPM)"));
	mMaxPhotonsPerPassProp = new IntProperty(tr("Max Photons per Pass"), 1000, 1, 10000000);
	mPPMGroupProp->addChild(mMaxPhotonsPerPassProp);
	mMaxPhotonPassCountProp = new IntProperty(tr("Max Pass Count"), 50, 1, 10000000);
	mPPMGroupProp->addChild(mMaxPhotonPassCountProp);
	mMaxPhotonGatherCountProp = new IntProperty(tr("Max Gather Count"), 500, 0);
	mPPMGroupProp->addChild(mMaxPhotonGatherCountProp);
	mMaxPhotonGatherRadiusProp = new DoubleProperty(tr("Max Gather Radius"), 0.1, 0, 10000);
	mPPMGroupProp->addChild(mMaxPhotonGatherRadiusProp);
	mPhotonGatheringModeProp = new SelectionProperty(tr("Gathering Mode"), PR::PGM_Sphere);
	((SelectionProperty*)mPhotonGatheringModeProp)->addItem(tr("Sphere"), PR::PGM_Sphere);
	((SelectionProperty*)mPhotonGatheringModeProp)->addItem(tr("Dome"), PR::PGM_Dome);
	mPPMGroupProp->addChild(mPhotonGatheringModeProp);
	mPhotonSqueezeWeightProp = new DoubleProperty(tr("Squeeze Weight"), 0, 0, 1);
	mPPMGroupProp->addChild(mPhotonSqueezeWeightProp);
	mProperties.add(mPPMGroupProp);

	mView->setPropertyTable(&mProperties);
	mView->expandToDepth(1);

	connect(&mProperties, SIGNAL(valueChanged(IProperty*)), this, SLOT(propertyValueChanged(IProperty*)));
}

SystemPropertyView::~SystemPropertyView()
{
	delete mRendererGroupProp;
	delete mRendererTileXProp;
	delete mRendererTileYProp;
	delete mRendererTileModeProp;
	delete mRendererThreadsProp;
	delete mRendererIncremental;
	delete mSamplerGroupProp;
	delete mSamplerAAProp;
	delete mSamplerAAMaxProp;
	delete mSamplerLensProp;
	delete mSamplerLensMaxProp;
	delete mSamplerTimeProp;
	delete mSamplerTimeMaxProp;
	delete mSamplerTimeMappingProp;
	delete mSamplerTimeScaleProp;
	delete mSamplerSpectralProp;
	delete mSamplerSpectralMaxProp;
	delete mRendererMaxDiffuseBouncesProp;
	delete mRendererMaxRayDepthProp;
	delete mIntegratorProp;
	delete mDebugVisualizationProp;
	delete mMaxLightSamplesProp;
	delete mMaxPhotonsPerPassProp;
	delete mMaxPhotonPassCountProp;
	delete mMaxPhotonGatherCountProp;
	delete mMaxPhotonGatherRadiusProp;
	delete mPhotonGatheringModeProp;
	delete mPhotonSqueezeWeightProp;
	delete mPPMGroupProp;
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

void SystemPropertyView::fillContent(PR::RenderFactory* renderer)
{
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setValue(0);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setDefaultValue(0);

	reinterpret_cast<SelectionProperty*>(mRendererTileModeProp)->setIndex(renderer->settings().tileMode());
	reinterpret_cast<SelectionProperty*>(mRendererTileModeProp)->setDefaultIndex(renderer->settings().tileMode());

	reinterpret_cast<BoolProperty*>(mRendererIncremental)->setValue(renderer->settings().isIncremental());
	reinterpret_cast<BoolProperty*>(mRendererIncremental)->setDefaultValue(renderer->settings().isIncremental());

	reinterpret_cast<SelectionProperty*>(mSamplerAAProp)->setIndex(renderer->settings().aaSampler());
	reinterpret_cast<SelectionProperty*>(mSamplerAAProp)->setDefaultIndex(renderer->settings().aaSampler());
	reinterpret_cast<IntProperty*>(mSamplerAAMaxProp)->setValue(renderer->settings().maxAASampleCount());
	reinterpret_cast<IntProperty*>(mSamplerAAMaxProp)->setDefaultValue(renderer->settings().maxAASampleCount());
	reinterpret_cast<SelectionProperty*>(mSamplerLensProp)->setIndex(renderer->settings().lensSampler());
	reinterpret_cast<SelectionProperty*>(mSamplerLensProp)->setDefaultIndex(renderer->settings().lensSampler());
	reinterpret_cast<IntProperty*>(mSamplerLensMaxProp)->setValue(renderer->settings().maxLensSampleCount());
	reinterpret_cast<IntProperty*>(mSamplerLensMaxProp)->setDefaultValue(renderer->settings().maxLensSampleCount());
	reinterpret_cast<SelectionProperty*>(mSamplerTimeProp)->setIndex(renderer->settings().timeSampler());
	reinterpret_cast<SelectionProperty*>(mSamplerTimeProp)->setDefaultIndex(renderer->settings().timeSampler());
	reinterpret_cast<IntProperty*>(mSamplerTimeMaxProp)->setValue(renderer->settings().maxTimeSampleCount());
	reinterpret_cast<IntProperty*>(mSamplerTimeMaxProp)->setDefaultValue(renderer->settings().maxTimeSampleCount());
	reinterpret_cast<SelectionProperty*>(mSamplerTimeMappingProp)->setIndex(renderer->settings().timeMappingMode());
	reinterpret_cast<SelectionProperty*>(mSamplerTimeMappingProp)->setDefaultIndex(renderer->settings().timeMappingMode());
	reinterpret_cast<DoubleProperty*>(mSamplerTimeScaleProp)->setValue(renderer->settings().timeScale());
	reinterpret_cast<DoubleProperty*>(mSamplerTimeScaleProp)->setDefaultValue(renderer->settings().timeScale());
	reinterpret_cast<SelectionProperty*>(mSamplerSpectralProp)->setIndex(renderer->settings().spectralSampler());
	reinterpret_cast<SelectionProperty*>(mSamplerSpectralProp)->setDefaultIndex(renderer->settings().spectralSampler());
	reinterpret_cast<IntProperty*>(mSamplerSpectralMaxProp)->setValue(renderer->settings().maxSpectralSampleCount());
	reinterpret_cast<IntProperty*>(mSamplerSpectralMaxProp)->setDefaultValue(renderer->settings().maxSpectralSampleCount());

	reinterpret_cast<IntProperty*>(mRendererMaxDiffuseBouncesProp)->setValue(renderer->settings().maxDiffuseBounces());
	reinterpret_cast<IntProperty*>(mRendererMaxDiffuseBouncesProp)->setDefaultValue(renderer->settings().maxDiffuseBounces());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setValue(renderer->settings().maxRayDepth());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setDefaultValue(renderer->settings().maxRayDepth());
	reinterpret_cast<SelectionProperty*>(mIntegratorProp)->setIndex(renderer->settings().integratorMode());
	reinterpret_cast<SelectionProperty*>(mIntegratorProp)->setDefaultIndex(renderer->settings().integratorMode());
	reinterpret_cast<SelectionProperty*>(mDebugVisualizationProp)->setIndex(renderer->settings().debugMode());
	reinterpret_cast<SelectionProperty*>(mDebugVisualizationProp)->setDefaultIndex(renderer->settings().debugMode());

	reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->setValue(renderer->settings().maxLightSamples());
	reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->setDefaultValue(renderer->settings().maxLightSamples());

	reinterpret_cast<IntProperty*>(mMaxPhotonsPerPassProp)->setValue(renderer->settings().ppm().maxPhotonsPerPass());
	reinterpret_cast<IntProperty*>(mMaxPhotonsPerPassProp)->setDefaultValue(renderer->settings().ppm().maxPhotonsPerPass());
	reinterpret_cast<IntProperty*>(mMaxPhotonPassCountProp)->setValue(renderer->settings().ppm().maxPassCount());
	reinterpret_cast<IntProperty*>(mMaxPhotonPassCountProp)->setDefaultValue(renderer->settings().ppm().maxPassCount());
	reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->setValue(renderer->settings().ppm().maxGatherRadius());
	reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->setDefaultValue(renderer->settings().ppm().maxGatherRadius());
	reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->setValue(renderer->settings().ppm().maxGatherCount());
	reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->setDefaultValue(renderer->settings().ppm().maxGatherCount());
	reinterpret_cast<SelectionProperty*>(mPhotonGatheringModeProp)->setIndex(renderer->settings().ppm().gatheringMode());
	reinterpret_cast<SelectionProperty*>(mPhotonGatheringModeProp)->setDefaultIndex(renderer->settings().ppm().gatheringMode());
	reinterpret_cast<DoubleProperty*>(mPhotonSqueezeWeightProp)->setValue(renderer->settings().ppm().squeezeWeight());
	reinterpret_cast<DoubleProperty*>(mPhotonSqueezeWeightProp)->setDefaultValue(renderer->settings().ppm().squeezeWeight());
}

void SystemPropertyView::setupRenderer(PR::RenderFactory* renderer)
{
	renderer->settings().setMaxDiffuseBounces(reinterpret_cast<IntProperty*>(mRendererMaxDiffuseBouncesProp)->value());
	renderer->settings().setMaxRayDepth(reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->value());
	renderer->settings().setIntegratorMode((PR::IntegratorMode)reinterpret_cast<SelectionProperty*>(mIntegratorProp)->index());
	renderer->settings().setDebugMode((PR::DebugMode)reinterpret_cast<SelectionProperty*>(mDebugVisualizationProp)->index());
	renderer->settings().setIncremental(reinterpret_cast<BoolProperty*>(mRendererIncremental)->value());
	renderer->settings().setTileMode((PR::TileMode)reinterpret_cast<SelectionProperty*>(mRendererTileModeProp)->index());

	renderer->settings().setAASampler((PR::SamplerMode)reinterpret_cast<SelectionProperty*>(mSamplerAAProp)->index());
	renderer->settings().setMaxAASampleCount(reinterpret_cast<IntProperty*>(mSamplerAAMaxProp)->value());
	renderer->settings().setLensSampler((PR::SamplerMode)reinterpret_cast<SelectionProperty*>(mSamplerLensProp)->index());
	renderer->settings().setMaxLensSampleCount(reinterpret_cast<IntProperty*>(mSamplerLensMaxProp)->value());
	renderer->settings().setTimeSampler((PR::SamplerMode)reinterpret_cast<SelectionProperty*>(mSamplerTimeProp)->index());
	renderer->settings().setMaxTimeSampleCount(reinterpret_cast<IntProperty*>(mSamplerTimeMaxProp)->value());
	renderer->settings().setTimeMappingMode((PR::TimeMappingMode)reinterpret_cast<SelectionProperty*>(mSamplerTimeMappingProp)->index());
	renderer->settings().setTimeScale(reinterpret_cast<DoubleProperty*>(mSamplerTimeScaleProp)->value());
	renderer->settings().setSpectralSampler((PR::SamplerMode)reinterpret_cast<SelectionProperty*>(mSamplerSpectralProp)->index());
	renderer->settings().setMaxSpectralSampleCount(reinterpret_cast<IntProperty*>(mSamplerSpectralMaxProp)->value());
	
	renderer->settings().setMaxLightSamples(reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->value());

	renderer->settings().ppm().setMaxPhotonsPerPass(reinterpret_cast<IntProperty*>(mMaxPhotonsPerPassProp)->value());
	renderer->settings().ppm().setMaxPassCount(reinterpret_cast<IntProperty*>(mMaxPhotonPassCountProp)->value());
	renderer->settings().ppm().setMaxGatherCount(reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->value());
	renderer->settings().ppm().setMaxGatherRadius(reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->value());
	renderer->settings().ppm().setGatheringMode((PR::PPMGatheringMode)reinterpret_cast<SelectionProperty*>(mPhotonGatheringModeProp)->index());
	renderer->settings().ppm().setSqueezeWeight(reinterpret_cast<DoubleProperty*>(mPhotonSqueezeWeightProp)->value());
}

