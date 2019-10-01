#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void about();
	void openWebsite();

	void openCNTFile();
	void openRDMPFile();
	void openRDMPDir();

private:
	void readSettings();
	void writeSettings();

	Ui::MainWindowClass ui;
	QString mLastDir;
};