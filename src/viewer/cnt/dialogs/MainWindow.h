#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include <memory>

class Container;
class GraphicObject;
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	void openProject(const QString& str);

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void depthChanged(int tick);

	void openScene();
	void about();
	void openWebsite();

private:
	void readSettings();
	void writeSettings();

	Ui::MainWindowClass ui;

	std::unique_ptr<Container> mContainer;
	std::shared_ptr<GraphicObject> mGraphicObject;
};