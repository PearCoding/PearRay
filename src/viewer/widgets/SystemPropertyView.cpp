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

	mRendererThreadsProp = new IntProperty(tr("Threads"), 0, 0, 64);
	mRendererThreadsProp->setToolTip(tr("0 = Automatic"));
	mRendererGroupProp->addChild(mRendererThreadsProp);

	mRendererMaxRayDepthProp = new IntProperty(tr("Max Ray Depth"), 10, 1, 4096);
	mRendererGroupProp->addChild(mRendererMaxRayDepthProp);

	mRendererStartProp = new ButtonProperty(tr("Start"));
	mRendererGroupProp->addChild(mRendererStartProp);

	mProperties.add(mRendererGroupProp);

	// Sampler
	mSamplerGroupProp = new GroupProperty(tr("Sampler"));

	mSamplerProp = new SelectionProperty(tr("Mode"), PR::SM_Jitter);
	((SelectionProperty*)mSamplerProp)->addItem(tr("None"), PR::SM_None);
	((SelectionProperty*)mSamplerProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mSamplerProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mSamplerProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	mSamplerGroupProp->addChild(mSamplerProp);

	mXSamplesProp = new IntProperty(tr("X Samples"), 8, 1, 1024);
	mSamplerGroupProp->addChild(mXSamplesProp);

	mYSamplesProp = new IntProperty(tr("Y Samples"), 8, 1, 1024);
	mSamplerGroupProp->addChild(mYSamplesProp);
	mProperties.add(mSamplerGroupProp);

	// Direct Lightning
	mDirectLightningGroupProp = new GroupProperty(tr("Direct Lightning"));
	mMaxLightSamplesProp = new IntProperty(tr("Max Light Samples"), 2);
	mDirectLightningGroupProp->addChild(mMaxLightSamplesProp);
	mProperties.add(mDirectLightningGroupProp);

	// Photon Mapping
	mPhotonMappingGroupProp = new GroupProperty(tr("Photon Mapping"));
	mMaxPhotonsProp = new IntProperty(tr("Max Photons"), 1000, 0, 10000000);
	mPhotonMappingGroupProp->addChild(mMaxPhotonsProp);
	mMaxPhotonGatherCountProp = new IntProperty(tr("Max Photon Gather Count"), 500, 0);
	mPhotonMappingGroupProp->addChild(mMaxPhotonGatherCountProp);
	mMaxPhotonGatherRadiusProp = new DoubleProperty(tr("Max Photon Gather Radius"), 0.1, 0);
	mPhotonMappingGroupProp->addChild(mMaxPhotonGatherRadiusProp);
	mProperties.add(mPhotonMappingGroupProp);

	// View
	mViewGroupProp = new GroupProperty(tr("View"));
	mViewModeProp = new SelectionProperty(tr("Display"), VM_Color);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Tone Mapped"), VM_ToneMapped);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Color"), VM_Color);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Color Linear"), VM_ColorLinear);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Depth"), VM_Depth);
	((SelectionProperty*)mViewModeProp)->addItem(tr("CIE XYZ"), VM_XYZ);
	((SelectionProperty*)mViewModeProp)->addItem(tr("CIE Norm XYZ"), VM_NORM_XYZ);
	mViewGroupProp->addChild(mViewModeProp);

	mViewScaleProp = new BoolProperty(tr("Scale to View"));
	mViewGroupProp->addChild(mViewScaleProp);
	mProperties.add(mViewGroupProp);

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
	delete mXSamplesProp;
	delete mYSamplesProp;
	delete mSamplerProp;
	delete mRendererMaxRayDepthProp;
	delete mMaxLightSamplesProp;
	delete mMaxPhotonsProp;
	delete mMaxPhotonGatherCountProp;
	delete mMaxPhotonGatherRadiusProp;
	delete mViewGroupProp;
	delete mViewModeProp;
	delete mViewScaleProp;
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
	if (prop == mRendererStartProp)
	{
		if (mRendererStartProp->propertyName() == tr("Stop"))
		{
			emit stopRendering();
		}
		else
		{
			emit startRendering();
		}
	}
	else if (prop == mViewModeProp)
	{
		emit viewModeChanged((ViewMode)((SelectionProperty*)mViewModeProp)->currentData().toInt());
	}
	else if (prop == mViewScaleProp)
	{
		emit viewScaleChanged(((BoolProperty*)mViewScaleProp)->value());
	}
}

void SystemPropertyView::enableRendering()
{
	mRendererTileXProp->setEnabled(false);
	mRendererTileYProp->setEnabled(false);
	mRendererThreadsProp->setEnabled(false);
	mRendererMaxRayDepthProp->setEnabled(false);

	mMaxLightSamplesProp->setEnabled(false);

	mMaxPhotonsProp->setEnabled(false);
	mMaxPhotonGatherRadiusProp->setEnabled(false);
	mMaxPhotonGatherCountProp->setEnabled(false);

	mXSamplesProp->setEnabled(false);
	mYSamplesProp->setEnabled(false);
	mSamplerProp->setEnabled(false);
	mRendererStartProp->setPropertyName(tr("Stop"));
}

void SystemPropertyView::disableRendering()
{
	mRendererTileXProp->setEnabled(true);
	mRendererTileYProp->setEnabled(true);
	mRendererThreadsProp->setEnabled(true);
	mRendererMaxRayDepthProp->setEnabled(true);

	mMaxLightSamplesProp->setEnabled(false);

	mMaxPhotonsProp->setEnabled(false);
	mMaxPhotonGatherRadiusProp->setEnabled(false);
	mMaxPhotonGatherCountProp->setEnabled(false);

	mXSamplesProp->setEnabled(true);
	mYSamplesProp->setEnabled(true);
	mSamplerProp->setEnabled(true);

	mRendererStartProp->setPropertyName(tr("Start"));
}

void SystemPropertyView::fillContent(PR::Renderer* renderer)
{
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setValue(0);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setDefaultValue(0);

	reinterpret_cast<IntProperty*>(mXSamplesProp)->setValue(renderer->settings().xSamplerCount());
	reinterpret_cast<IntProperty*>(mXSamplesProp)->setDefaultValue(renderer->settings().xSamplerCount());
	reinterpret_cast<IntProperty*>(mYSamplesProp)->setValue(renderer->settings().ySamplerCount());
	reinterpret_cast<IntProperty*>(mYSamplesProp)->setDefaultValue(renderer->settings().ySamplerCount());
	reinterpret_cast<SelectionProperty*>(mSamplerProp)->setIndex(renderer->settings().samplerMode());
	reinterpret_cast<SelectionProperty*>(mSamplerProp)->setDefaultIndex(renderer->settings().samplerMode());

	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setValue(renderer->settings().maxRayDepth());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setDefaultValue(renderer->settings().maxRayDepth());

	reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->setValue(renderer->settings().maxLightSamples());
	reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->setDefaultValue(renderer->settings().maxLightSamples());

	reinterpret_cast<IntProperty*>(mMaxPhotonsProp)->setValue(renderer->settings().maxPhotons());
	reinterpret_cast<IntProperty*>(mMaxPhotonsProp)->setDefaultValue(renderer->settings().maxPhotons());
	reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->setValue(
		renderer->settings().maxPhotonGatherRadius());
	reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->setDefaultValue(
		renderer->settings().maxPhotonGatherRadius());
	reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->setValue(renderer->settings().maxPhotonGatherCount());
	reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->setDefaultValue(renderer->settings().maxPhotonGatherCount());
}

void SystemPropertyView::setupRenderer(PR::Renderer* renderer)
{
	renderer->settings().setMaxRayDepth(reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->value());

	renderer->settings().setSamplerMode((PR::SamplerMode)reinterpret_cast<SelectionProperty*>(mSamplerProp)->index());
	renderer->settings().setXSamplerCount(reinterpret_cast<IntProperty*>(mXSamplesProp)->value());
	renderer->settings().setYSamplerCount(reinterpret_cast<IntProperty*>(mYSamplesProp)->value());

	renderer->settings().setMaxLightSamples(reinterpret_cast<IntProperty*>(mMaxLightSamplesProp)->value());

	renderer->settings().setMaxPhotons(reinterpret_cast<IntProperty*>(mMaxPhotonsProp)->value());
	renderer->settings().setMaxPhotonGatherCount(reinterpret_cast<IntProperty*>(mMaxPhotonGatherCountProp)->value());
	renderer->settings().setMaxPhotonGatherRadius(reinterpret_cast<DoubleProperty*>(mMaxPhotonGatherRadiusProp)->value());
}

