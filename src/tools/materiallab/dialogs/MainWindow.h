#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QPointer>
#include <memory>

namespace PR {
class Environment;
}

class SceneWindow;
class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

	void newInspection(const QString& name);
	
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
	std::shared_ptr<PR::Environment> mEnv;
};