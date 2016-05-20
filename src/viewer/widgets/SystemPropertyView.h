#pragma once

#include <QWidget>
#include "properties/PropertyTable.h"
#include "ViewWidget.h"

namespace PR
{
	class Renderer;
}

class IProperty;
class PropertyView;
class SystemPropertyView : public QWidget
{
	Q_OBJECT
public:
	explicit SystemPropertyView(QWidget* parent = nullptr);
	virtual ~SystemPropertyView();

	void enableRendering();
	void disableRendering();

	int getTileX() const;
	int getTileY() const;
	int getThreadCount() const;

	void fillContent(PR::Renderer* renderer);
	void setupRenderer(PR::Renderer* renderer);
signals:
	void startRendering();
	void stopRendering();
	void viewModeChanged(ViewMode);
	void viewScaleChanged(bool);

private slots:
	void propertyValueChanged(IProperty* prop);

private:
	PropertyView* mView;

	PropertyTable mProperties;
	IProperty* mRendererGroupProp;
	IProperty* mRendererTileXProp;
	IProperty* mRendererTileYProp;
	IProperty* mRendererThreadsProp;

	IProperty* mRendererMaxDiffuseBouncesProp;
	IProperty* mRendererMaxRayDepthProp;
	IProperty* mDebugVisualizationProp;

	IProperty* mSamplerGroupProp;
	IProperty* mSamplerProp;
	IProperty* mXSamplesProp;
	IProperty* mYSamplesProp;

	IProperty* mDirectLightningGroupProp;
	IProperty* mMaxLightSamplesProp;
	IProperty* mUseBiDirectProp;

	IProperty* mPhotonMappingGroupProp;
	IProperty* mMaxPhotonsProp;
	IProperty* mMaxPhotonGatherRadiusProp;
	IProperty* mMaxPhotonGatherCountProp;
	IProperty* mMaxPhotonDiffuseBouncesProp;
	IProperty* mMinPhotonSpecularBouncesProp;
	IProperty* mPhotonGatheringModeProp;

	IProperty* mRendererStartProp;// Button
	IProperty* mViewGroupProp;
	IProperty* mViewModeProp;
	IProperty* mViewScaleProp;
};
