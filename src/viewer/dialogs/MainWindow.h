#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include "properties/PropertyTable.h"

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
	void stopRendering();

	void entitySelected(QModelIndex index);

private:
	void readSettings();
	void writeSettings();
	
	Ui::MainWindowClass ui;

	PRU::Environment* mEnvironment;
	PR::Renderer* mRenderer;

	QTimer mTimer;
};