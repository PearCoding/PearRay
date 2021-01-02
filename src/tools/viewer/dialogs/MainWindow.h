#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QPointer>
#include <QTimer>

class Project;
class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

	void openFile(const QString& file);

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void about();
	void openWebsite();

	void openFile();

	void openRecentFile();

	void exportImage();
	void updatePipeline();

	void startStopRender();

	void renderingStarted();
	void renderingFinished();

private:
	void readSettings();
	void writeSettings();

	void setupRecentMenu();
	void setupDockWidgets();
	void setupToolbars();

	void addToRecentFiles(const QString& path);
	void updateRecentFiles();

	void setupProjectContext();
	void closeProject();

	Ui::MainWindowClass ui;
	QString mLastDir;

	QStringList mLastFiles;
	QVector<QAction*> mLastFileActions;

	QTimer mImageTimer;
	QPointer<Project> mProject;

	// Some settings
	int mImageUpdateIntervalMSecs;
};