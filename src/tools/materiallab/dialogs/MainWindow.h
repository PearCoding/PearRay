#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QPointer>

class SceneWindow;
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

private:
	void readSettings();
	void writeSettings();

	Ui::MainWindowClass ui;
};