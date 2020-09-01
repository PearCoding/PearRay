#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QPointer>

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
private:
	void readSettings();
	void writeSettings();

	void setupRecentMenu();

	void addToRecentFiles(const QString& path);
	void updateRecentFiles();

	Ui::MainWindowClass ui;
	QString mLastDir;

	QStringList mLastFiles;
	QVector<QAction*> mLastFileActions;
};