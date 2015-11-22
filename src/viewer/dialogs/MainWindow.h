#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include "properties/propertytable.h"

#include <QTimer>

namespace PR
{
	class Renderer;
}

namespace PRU
{
	class Environment;
}

class IProperty;
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	
protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void showAllToolbars();
	void hideAllToolbars();

	void showAllDocks();
	void hideAllDocks();

	void about();
	void openWebsite();

	void updateView();

	void propertyValueChanged(IProperty* prop);
private:
	void readSettings();
	void writeSettings();

	void startRendering();
	void stopRendering();
	
	Ui::MainWindowClass ui;

	PRU::Environment* mEnvironment;
	PR::Renderer* mRenderer;

	PropertyTable mProperties;
	IProperty* mRendererGroupProp;
	IProperty* mRendererTileXProp;
	IProperty* mRendererTileYProp;
	IProperty* mRendererThreadsProp;
	IProperty* mRendererMaxRayDepthProp;
	IProperty* mRendererMaxDirectRayCountProp;
	IProperty* mRendererMaxIndirectRayCountProp;
	IProperty* mRendererSamplingProp;
	IProperty* mRendererSamplesPerRayProp;
	IProperty* mRendererSamplerProp;
	IProperty* mRendererStartProp;// Button
	IProperty* mViewGroupProp;
	IProperty* mViewModeProp;

	QTimer mTimer;
};