#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

#include <QFutureWatcher>
#include <QSignalMapper>
#include <QLabel>

#include "Config.h"

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
private:
	void readSettings();
	void writeSettings();
	
	Ui::MainWindowClass ui;
};