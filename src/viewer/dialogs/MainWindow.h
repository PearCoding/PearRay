#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include "properties/PropertyTable.h"

#include <QTimer>
#include <QElapsedTimer>

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
	
	void openProject(const QString& str);
	void closeProject();

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void openScene();
	void exportImage();

	void showAllToolbars();
	void hideAllToolbars();

	void showAllDocks();
	void hideAllDocks();

	void about();
	void openWebsite();

	void updateView();

	void startRendering();
	void restartRendering();
	void stopRendering();

	void entitySelected(QModelIndex index);

	void setViewColorMode(int);
	void setViewGammaMode(int);
	void setViewMapperMode(int);

	void selectSelectionTool(bool b);
	void selectPanTool(bool b);
	void selectZoomTool(bool b);
	void selectCropTool(bool b);

private:
	void startRendering(bool clear);
	void readSettings();
	void writeSettings();
	
	Ui::MainWindowClass ui;

	PRU::Environment* mEnvironment;
	PR::Renderer* mRenderer;

	QTimer mTimer;
	QElapsedTimer mElapsedTime;
	QElapsedTimer mFrameTime;
	float mLastPercent;
};