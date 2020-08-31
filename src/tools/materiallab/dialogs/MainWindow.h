#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QPointer>
#include <memory>

namespace PR {
class QueryEnvironment;
}

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

	void newInspection();

private:
	void readSettings();
	void writeSettings();

	Ui::MainWindowClass ui;
	std::unique_ptr<PR::QueryEnvironment> mEnv;
};