#pragma once

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWinTaskbarButton)
QT_FORWARD_DECLARE_CLASS(QWinTaskbarProgress)
QT_FORWARD_DECLARE_CLASS(QWinThumbnailToolBar)
QT_FORWARD_DECLARE_CLASS(QWinThumbnailToolButton)

/// Class which hides the dependency to windows extra features
class WinExtraWidget : public QObject {
	Q_OBJECT

public:
	WinExtraWidget(QWidget* view);
	~WinExtraWidget();

	void setWindow(QWidget* view);

	static bool isSupported();

public slots:
	void enableProject(bool b);
	void renderingStarted();
	void renderingFinished();
	void setProgressValue(int i);
	void setProgressRange(int start, int end);
	void setThumbnail(const QPixmap& pixmap);
	void setLivePreview(const QPixmap& pixmap);

signals:
	void startStopRequested();
	void thumbnailRequested();

private:
	QWinTaskbarButton* mTaskbar;
	QWinTaskbarProgress* mProgress;
	QWinThumbnailToolBar* mToolBar;
	QWinThumbnailToolButton* mStartStopButton;
};