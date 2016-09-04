#pragma once

#include <QWidget>
#include "properties/PropertyTable.h"

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

	int getTileX() const;
	int getTileY() const;
	int getThreadCount() const;

	void fillContent(PR::Renderer* renderer);
	void setupRenderer(PR::Renderer* renderer);
signals:
	void startRendering();
	void stopRendering();

private slots:
	void propertyValueChanged(IProperty* prop);

private:
	PropertyView* mView;

	PropertyTable mProperties;
	IProperty* mRendererGroupProp;
	IProperty* mRendererTileXProp;
	IProperty* mRendererTileYProp;
	IProperty* mRendererThreadsProp;
	IProperty* mRendererIncremental;

	IProperty* mRendererMaxDiffuseBouncesProp;
	IProperty* mRendererMaxRayDepthProp;
	IProperty* mIntegratorProp;
	IProperty* mDebugVisualizationProp;

	IProperty* mPixelSamplerGroupProp;
	IProperty* mPixelSamplerProp;
	IProperty* mPixelSamplesProp;

	IProperty* mDirectLightningGroupProp;
	IProperty* mMaxLightSamplesProp;

	IProperty* mPPMGroupProp;
	IProperty* mMaxPhotonsPerPassProp;
	IProperty* mMaxPhotonPassCountProp;
	IProperty* mMaxPhotonGatherRadiusProp;
	IProperty* mMaxPhotonGatherCountProp;
	IProperty* mPhotonGatheringModeProp;
	IProperty* mPhotonSqueezeWeightProp;
};
