#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include "properties/propertytable.h"

#include <QTimer>

namespace PR
{
	class Camera;
	class DiffuseMaterial;
	class Scene;
	class Renderer;
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

	PR::Camera* mCamera;
	PR::Scene* mScene;
	PR::Renderer* mRenderer;

	PR::DiffuseMaterial* mObjectMaterial;
	PR::DiffuseMaterial* mLightMaterial;

	PropertyTable mProperties;
	IProperty* mRendererGroupProp;
	IProperty* mRendererTileXProp;
	IProperty* mRendererTileYProp;
	IProperty* mRendererThreadsProp;
	IProperty* mRendererMaxRayDepthProp;
	IProperty* mRendererMaxBounceRayCountProp;
	IProperty* mRendererSubPixelsProp;
	IProperty* mRendererStartProp;// Button

	QTimer mTimer;
};