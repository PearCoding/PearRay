#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

class Container;
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

	Container* mContainer;
};