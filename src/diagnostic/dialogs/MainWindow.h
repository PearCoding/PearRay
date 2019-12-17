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

	void openFile(const QString& file);
	void openRDMPDir(const QString& file);

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void about();
	void openWebsite();

	void openFile();
	void openRDMPDir();

private:
	void readSettings();
	void writeSettings();

	void setupSceneWindow();

	void setupRecentMenu();

	void updateRecentFiles(const QString& path);
	void setupRecentFiles();

	void updateRecentDirs(const QString& path);
	void setupRecentDirs();

	Ui::MainWindowClass ui;
	QString mLastDir;

	QStringList mLastFiles;
	QStringList mLastDirs;
	QVector<QAction*> mLastFileActions;
	QVector<QAction*> mLastDirActions;

	QPointer<SceneWindow> mCurrentSceneWindow;
};