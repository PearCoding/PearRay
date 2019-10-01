#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include <memory>

class GraphicObject;
class RayArray;
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	void openProject(const QString& str);
	void openProjectDir(const QString& str);

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void openDump();
	void openMultipleDumps();
	void about();
	void openWebsite();

private:
	void readSettings();
	void writeSettings();

	Ui::MainWindowClass ui;

	std::unique_ptr<RayArray> mRays;
	std::shared_ptr<GraphicObject> mGraphicObject;

	QString mLastDir;
};