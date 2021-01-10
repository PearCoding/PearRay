#include "WinExtraWidget.h"

#include <QWidget>

#ifdef PR_UI_HAS_WINEXTRAS
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#endif

WinExtraWidget::WinExtraWidget(QWidget* view)
	: QObject(view)
	, mTaskbar(nullptr)
	, mProgress(nullptr)
	, mToolBar(nullptr)
	, mStartStopButton(nullptr)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mTaskbar  = new QWinTaskbarButton(view);
	mProgress = mTaskbar->progress();

	mToolBar = new QWinThumbnailToolBar(view);
	connect(mToolBar, SIGNAL(iconicLivePreviewPixmapRequested()), this, SIGNAL(thumbnailRequested()));
	connect(mToolBar, SIGNAL(iconicThumbnailPixmapRequested()), this, SIGNAL(thumbnailRequested()));

	mStartStopButton = new QWinThumbnailToolButton(mToolBar);
	mStartStopButton->setEnabled(false);
	connect(mStartStopButton, SIGNAL(clicked()), this, SIGNAL(startStopRequested()));

	mToolBar->addButton(mStartStopButton);
#endif

	// Init
	renderingFinished();
}

WinExtraWidget::~WinExtraWidget()
{
}

bool WinExtraWidget::isSupported()
{
#ifdef PR_UI_HAS_WINEXTRAS
	return true;
#else
	return false;
#endif
}

void WinExtraWidget::setWindow(QWidget* view)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mTaskbar->setWindow(view->windowHandle());
	mToolBar->setWindow(view->windowHandle());
	mToolBar->setIconicPixmapNotificationsEnabled(true);
#else
	Q_UNUSED(view);
#endif
}

void WinExtraWidget::enableProject(bool b)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mStartStopButton->setEnabled(b);
#else
	Q_UNUSED(b);
#endif
}

void WinExtraWidget::renderingStarted()
{
#ifdef PR_UI_HAS_WINEXTRAS
	mProgress->show();
	mProgress->resume();
	mStartStopButton->setIcon(QIcon(":/stop_icon"));
	mStartStopButton->setToolTip(tr("Stop"));
#endif
}

void WinExtraWidget::renderingFinished()
{
#ifdef PR_UI_HAS_WINEXTRAS
	mProgress->stop();
	if (mProgress->value() == mProgress->maximum())
		mProgress->hide();
	mStartStopButton->setIcon(QIcon(":/play_icon"));
	mStartStopButton->setToolTip(tr("Start"));
#endif
}

void WinExtraWidget::setProgressValue(int i)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mProgress->setValue(i);
#else
	Q_UNUSED(i);
#endif
}

void WinExtraWidget::setProgressRange(int start, int end)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mProgress->setRange(start, end);
#else
	Q_UNUSED(start);
	Q_UNUSED(end);
#endif
}

void WinExtraWidget::setThumbnail(const QPixmap& pixmap)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mToolBar->setIconicThumbnailPixmap(pixmap);
#else
	Q_UNUSED(pixmap);
#endif
}

void WinExtraWidget::setLivePreview(const QPixmap& pixmap)
{
#ifdef PR_UI_HAS_WINEXTRAS
	mToolBar->setIconicLivePreviewPixmap(pixmap);
#else
	Q_UNUSED(pixmap);
#endif
}