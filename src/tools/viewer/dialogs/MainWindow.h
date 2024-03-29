#pragma once

#include "ui_MainWindow.h"
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QProgressBar>
#include <QSpinBox>
#include <QTimer>

#include "log/LogTableModel.h"
#include "log/UILogListener.h"
#include "widgets/WinExtraWidget.h"

class Project;
class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

	void openFile(const QString& file);

protected:
	void closeEvent(QCloseEvent* event);
	void showEvent(QShowEvent* event);

private slots:
	void about();
	void openWebsite();

	void openFile();

	void openRecentFile();

	void exportImage();
	void updatePipeline();
	void updateImage();
	void updateThumbnail();

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
	void clearData();

	void updateStatus(bool running);
	void updateProgress(int iteration);
	void updateRenderTime(bool running);
	void updateRenderStatus();

	static QPixmap pixmapFromSVG(const QString& filename, const QSize& baseSize);

	Ui::MainWindowClass ui;
	QString mLastDir;

	QStringList mLastFiles;
	QVector<QAction*> mLastFileActions;

	QTimer mImageTimer;
	QPointer<Project> mProject;

	QLabel* mRenderingIteration;
	QLabel* mRenderingTime;
	QLabel* mRenderingStatus;
	QProgressBar* mRenderingProgress;

	QSpinBox* mIterationOverride;

	WinExtraWidget* mWinExtras;

	PR::UI::LogTableModel mLog;
	std::shared_ptr<PR::UI::UILogListener> mLogListener;

	// Some settings
	int mImageUpdateIntervalMSecs;
};