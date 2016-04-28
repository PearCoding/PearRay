#include "SystemPropertyView.h"
#include "PropertyView.h"

#include "properties/PropertyTable.h"

#include "properties/GroupProperty.h"
#include "properties/IntProperty.h"
#include "properties/ButtonProperty.h"
#include "properties/BoolProperty.h"
#include "properties/SelectionProperty.h"

#include "renderer/Renderer.h"

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
	mRendererGroupProp = new GroupProperty();
	mRendererGroupProp->setPropertyName(tr("Renderer"));

	mRendererTileXProp = new IntProperty();
	mRendererTileXProp->setPropertyName(tr("Tile X Count"));
	((IntProperty*)mRendererTileXProp)->setMinValue(1);
	((IntProperty*)mRendererTileXProp)->setMaxValue(128);
	((IntProperty*)mRendererTileXProp)->setDefaultValue(5);
	((IntProperty*)mRendererTileXProp)->setValue(5);
	mRendererGroupProp->addChild(mRendererTileXProp);

	mRendererTileYProp = new IntProperty();
	mRendererTileYProp->setPropertyName(tr("Tile Y Count"));
	((IntProperty*)mRendererTileYProp)->setMinValue(1);
	((IntProperty*)mRendererTileYProp)->setMaxValue(128);
	((IntProperty*)mRendererTileYProp)->setDefaultValue(5);
	((IntProperty*)mRendererTileYProp)->setValue(5);
	mRendererGroupProp->addChild(mRendererTileYProp);

	mRendererThreadsProp = new IntProperty();
	mRendererThreadsProp->setPropertyName(tr("Threads"));
	mRendererThreadsProp->setToolTip(tr("0 = Automatic"));
	((IntProperty*)mRendererThreadsProp)->setMinValue(0);
	((IntProperty*)mRendererThreadsProp)->setMaxValue(64);
	((IntProperty*)mRendererThreadsProp)->setDefaultValue(0);
	((IntProperty*)mRendererThreadsProp)->setValue(0);
	mRendererGroupProp->addChild(mRendererThreadsProp);

	mRendererSamplingProp = new BoolProperty();
	mRendererSamplingProp->setPropertyName(tr("Sampling"));
	mRendererGroupProp->addChild(mRendererSamplingProp);

	mRendererXSamplesProp = new IntProperty();
	mRendererXSamplesProp->setPropertyName(tr("X Samples"));
	((IntProperty*)mRendererXSamplesProp)->setMinValue(1);
	((IntProperty*)mRendererXSamplesProp)->setMaxValue(1024);
	((IntProperty*)mRendererXSamplesProp)->setDefaultValue(8);
	((IntProperty*)mRendererXSamplesProp)->setValue(8);
	mRendererGroupProp->addChild(mRendererXSamplesProp);

	mRendererYSamplesProp = new IntProperty();
	mRendererYSamplesProp->setPropertyName(tr("Y Samples"));
	((IntProperty*)mRendererYSamplesProp)->setMinValue(1);
	((IntProperty*)mRendererYSamplesProp)->setMaxValue(1024);
	((IntProperty*)mRendererYSamplesProp)->setDefaultValue(8);
	((IntProperty*)mRendererYSamplesProp)->setValue(8);
	mRendererGroupProp->addChild(mRendererYSamplesProp);

	mRendererSamplerProp = new SelectionProperty();
	mRendererSamplerProp->setPropertyName(tr("Sampler"));
	((SelectionProperty*)mRendererSamplerProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mRendererSamplerProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mRendererSamplerProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mRendererSamplerProp)->setDefaultIndex(2);
	((SelectionProperty*)mRendererSamplerProp)->setIndex(2);
	mRendererGroupProp->addChild(mRendererSamplerProp);

	mRendererMaxRayDepthProp = new IntProperty();
	mRendererMaxRayDepthProp->setPropertyName(tr("Max Ray Depth"));
	((IntProperty*)mRendererMaxRayDepthProp)->setMinValue(1);
	((IntProperty*)mRendererMaxRayDepthProp)->setMaxValue(128);
	((IntProperty*)mRendererMaxRayDepthProp)->setDefaultValue(2);
	((IntProperty*)mRendererMaxRayDepthProp)->setValue(2);
	mRendererGroupProp->addChild(mRendererMaxRayDepthProp);

	mRendererMaxDirectRayCountProp = new IntProperty();
	mRendererMaxDirectRayCountProp->setPropertyName(tr("Max Direct Ray Count"));
	((IntProperty*)mRendererMaxDirectRayCountProp)->setMinValue(0);
	((IntProperty*)mRendererMaxDirectRayCountProp)->setMaxValue(999999);
	((IntProperty*)mRendererMaxDirectRayCountProp)->setDefaultValue(50);
	((IntProperty*)mRendererMaxDirectRayCountProp)->setValue(50);
	mRendererGroupProp->addChild(mRendererMaxDirectRayCountProp);

	mRendererMaxIndirectRayCountProp = new IntProperty();
	mRendererMaxIndirectRayCountProp->setPropertyName(tr("Max Indirect Ray Count"));
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setMinValue(0);
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setMaxValue(999999);
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setDefaultValue(100);
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setValue(100);
	mRendererGroupProp->addChild(mRendererMaxIndirectRayCountProp);

	mRendererStartProp = new ButtonProperty();
	mRendererStartProp->setPropertyName(tr("Start"));
	mRendererGroupProp->addChild(mRendererStartProp);

	mProperties.add(mRendererGroupProp);

	mViewGroupProp = new GroupProperty();
	mViewGroupProp->setPropertyName(tr("View"));

	mViewModeProp = new SelectionProperty();
	mViewModeProp->setPropertyName(tr("Display"));
	((SelectionProperty*)mViewModeProp)->addItem(tr("Color"), VM_Color);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Color Linear"), VM_ColorLinear);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Depth"), VM_Depth);
	((SelectionProperty*)mViewModeProp)->addItem(tr("CIE XYZ"), VM_XYZ);
	((SelectionProperty*)mViewModeProp)->addItem(tr("CIE Norm XYZ"), VM_NORM_XYZ);
	((SelectionProperty*)mViewModeProp)->setDefaultIndex(0);
	mViewGroupProp->addChild(mViewModeProp);

	mViewScaleProp = new BoolProperty();
	mViewScaleProp->setPropertyName(tr("Scale to View"));
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
	delete mRendererSamplingProp;
	delete mRendererXSamplesProp;
	delete mRendererYSamplesProp;
	delete mRendererSamplerProp;
	delete mRendererMaxRayDepthProp;
	delete mRendererMaxDirectRayCountProp;
	delete mRendererMaxIndirectRayCountProp;
	delete mViewGroupProp;
	delete mViewModeProp;
	delete mViewScaleProp;
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
	mRendererSamplingProp->setEnabled(false);
	mRendererXSamplesProp->setEnabled(false);
	mRendererYSamplesProp->setEnabled(false);
	mRendererSamplerProp->setEnabled(false);
	mRendererMaxDirectRayCountProp->setEnabled(false);
	mRendererMaxIndirectRayCountProp->setEnabled(false);
	mRendererMaxRayDepthProp->setEnabled(false);
	mRendererStartProp->setPropertyName(tr("Stop"));
}

void SystemPropertyView::disableRendering()
{
	mRendererTileXProp->setEnabled(true);
	mRendererTileYProp->setEnabled(true);
	mRendererThreadsProp->setEnabled(true);
	mRendererSamplingProp->setEnabled(true);
	mRendererXSamplesProp->setEnabled(true);
	mRendererYSamplesProp->setEnabled(true);
	mRendererSamplerProp->setEnabled(true);
	mRendererMaxDirectRayCountProp->setEnabled(true);
	mRendererMaxIndirectRayCountProp->setEnabled(true);
	mRendererMaxRayDepthProp->setEnabled(true);
	mRendererStartProp->setPropertyName(tr("Start"));
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

int SystemPropertyView::getMaxRayDepth() const
{
	return reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->value();
}

int SystemPropertyView::getMaxDirectRayCount() const
{
	return reinterpret_cast<IntProperty*>(mRendererMaxDirectRayCountProp)->value();
}

int SystemPropertyView::getMaxIndirectRayCount() const
{
	return reinterpret_cast<IntProperty*>(mRendererTileXProp)->value();
}

bool SystemPropertyView::getSampling() const
{
	return reinterpret_cast<BoolProperty*>(mRendererSamplingProp)->value();
}

int SystemPropertyView::getXSamples() const
{
	return reinterpret_cast<IntProperty*>(mRendererXSamplesProp)->value();
}

int SystemPropertyView::getYSamples() const
{
	return reinterpret_cast<IntProperty*>(mRendererYSamplesProp)->value();
}

int SystemPropertyView::getSampler() const
{
	return reinterpret_cast<SelectionProperty*>(mRendererSamplerProp)->index();
}

void SystemPropertyView::fillContent(PR::Renderer* renderer)
{
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileXProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setValue(8);
	reinterpret_cast<IntProperty*>(mRendererTileYProp)->setDefaultValue(8);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setValue(0);
	reinterpret_cast<IntProperty*>(mRendererThreadsProp)->setDefaultValue(0);

	reinterpret_cast<BoolProperty*>(mRendererSamplingProp)->setValue(renderer->isSamplingEnabled());
	reinterpret_cast<BoolProperty*>(mRendererSamplingProp)->setDefaultValue(renderer->isSamplingEnabled());
	reinterpret_cast<IntProperty*>(mRendererXSamplesProp)->setValue(renderer->xSampleCount());
	reinterpret_cast<IntProperty*>(mRendererXSamplesProp)->setDefaultValue(renderer->xSampleCount());
	reinterpret_cast<IntProperty*>(mRendererYSamplesProp)->setValue(renderer->ySampleCount());
	reinterpret_cast<IntProperty*>(mRendererYSamplesProp)->setDefaultValue(renderer->ySampleCount());
	reinterpret_cast<SelectionProperty*>(mRendererSamplerProp)->setIndex(renderer->samplerMode());
	reinterpret_cast<SelectionProperty*>(mRendererSamplerProp)->setDefaultIndex(renderer->samplerMode());
	reinterpret_cast<IntProperty*>(mRendererMaxDirectRayCountProp)->setValue(renderer->maxDirectRayCount());
	reinterpret_cast<IntProperty*>(mRendererMaxDirectRayCountProp)->setDefaultValue(renderer->maxDirectRayCount());
	reinterpret_cast<IntProperty*>(mRendererMaxIndirectRayCountProp)->setValue(renderer->maxIndirectRayCount());
	reinterpret_cast<IntProperty*>(mRendererMaxIndirectRayCountProp)->setDefaultValue(renderer->maxIndirectRayCount());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setValue(renderer->maxRayDepth());
	reinterpret_cast<IntProperty*>(mRendererMaxRayDepthProp)->setDefaultValue(renderer->maxRayDepth());
}