#pragma once

#include <QWidget>
#include "properties/PropertyTable.h"
#include "ViewWidget.h"

namespace PR
{
	class Entity;
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
	int getMaxRayDepth() const;
	int getMaxDirectRayCount() const;
	int getMaxIndirectRayCount() const;
	bool getSampling() const;
	int getXSamples() const;
	int getYSamples() const;
	int getSampler() const;

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
	IProperty* mRendererMaxRayDepthProp;
	IProperty* mRendererMaxDirectRayCountProp;
	IProperty* mRendererMaxIndirectRayCountProp;
	IProperty* mRendererSamplingProp;
	IProperty* mRendererXSamplesProp;
	IProperty* mRendererYSamplesProp;
	IProperty* mRendererSamplerProp;
	IProperty* mRendererStartProp;// Button
	IProperty* mViewGroupProp;
	IProperty* mViewModeProp;
	IProperty* mViewScaleProp;
};
