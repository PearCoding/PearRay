#include "MainWindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QFileDialog>
#include <QImageWriter>

#include "scene/Scene.h"
#include "camera/Camera.h"
#include "renderer/Renderer.h"

#include "Environment.h"
#include "SceneLoader.h"

#include "models/EntityTreeModel.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	mEnvironment(nullptr), mRenderer(nullptr)
{
	ui.setupUi(this);

	closeProject();// Initial

	//Add tool-bars to menu

	//Add dock widgets to menu
	ui.menuDockWidgets->addAction(ui.systemPropertyDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.spectrumDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.outlineDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.entityDetailsDockWidget->toggleViewAction());

	//Connect all signal and slots
	connect(ui.actionOpenScene, SIGNAL(triggered()), this, SLOT(openScene()));
	connect(ui.actionExportImage, SIGNAL(triggered()), this, SLOT(exportImage()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	
	connect(ui.actionShowToolbars, SIGNAL(triggered()), this, SLOT(showAllToolbars()));
	connect(ui.actionHideToolbars, SIGNAL(triggered()), this, SLOT(hideAllToolbars()));
	connect(ui.actionShowDockWidgets, SIGNAL(triggered()), this, SLOT(showAllDocks()));
	connect(ui.actionHideDockWidgets, SIGNAL(triggered()), this, SLOT(hideAllDocks()));

	connect(ui.systemPropertyView, SIGNAL(startRendering()), this, SLOT(startRendering()));
	connect(ui.systemPropertyView, SIGNAL(stopRendering()), this, SLOT(stopRendering()));
	connect(ui.systemPropertyView, SIGNAL(viewModeChanged(ViewMode)), ui.viewWidget, SLOT(setViewMode(ViewMode)));
	connect(ui.systemPropertyView, SIGNAL(viewScaleChanged(bool)), ui.viewWidget, SLOT(enableScale(bool)));

	connect(ui.outlineView, SIGNAL(activated(QModelIndex)), this, SLOT(entitySelected(QModelIndex)));

	connect(&mTimer, SIGNAL(timeout()), this, SLOT(updateView()));
	mTimer.setSingleShot(false);

	connect(ui.viewWidget, SIGNAL(spectrumSelected(const PR::Spectrum&)), ui.spectrumWidget, SLOT(setSpectrum(const PR::Spectrum)));

	readSettings();
}

MainWindow::~MainWindow()
{
	closeProject();
}

void MainWindow::closeProject()
{
	ui.outlineView->setEnabled(false);
	ui.viewWidget->setEnabled(false);
	ui.entityDetailsView->setEnabled(false);
	ui.systemPropertyView->setEnabled(false);

	ui.outlineView->setModel(nullptr);
	ui.viewWidget->setRenderer(nullptr);
	ui.entityDetailsView->setEntity(nullptr);

	if (mRenderer)
	{
		mRenderer->stop();
		mRenderer->waitForFinish();
		delete mRenderer;
		mRenderer = nullptr;
	}

	if (mEnvironment)
	{
		delete mEnvironment;
		mEnvironment = nullptr;
	}
}

void MainWindow::openProject(const QString& str)
{
	if (mEnvironment)
	{
		closeProject();
	}

	PRU::SceneLoader loader;
	mEnvironment = loader.loadFromFile(str.toStdString());

	if (mEnvironment)
	{
		mRenderer = new PR::Renderer(800, 600, mEnvironment->camera(), mEnvironment->scene());

		ui.viewWidget->setRenderer(mRenderer);
		ui.outlineView->setModel(new EntityTreeModel(mEnvironment->scene(), this));

		ui.systemPropertyView->fillContent(mRenderer);

		ui.outlineView->setEnabled(true);
		ui.viewWidget->setEnabled(true);
		ui.entityDetailsView->setEnabled(true);
		ui.systemPropertyView->setEnabled(true);
	}
	else
	{
		QMessageBox::warning(this, tr("Couldn't load project."), tr("Couldn't load project file:\n%1").arg(str));
	}
}

void MainWindow::openScene()
{
	if (mEnvironment && mRenderer && !mRenderer->isFinished())
	{
		if (QMessageBox::question(this, tr("Abort render?"), tr("A rendering is currently running.\nShould it be aborted?")) != QMessageBox::Yes)
		{
			return;
		}
	}

	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	QString file = QFileDialog::getOpenFileName(this, tr("Open Scene"),
		docLoc.isEmpty() ? QDir::currentPath() : docLoc.last());

	if (!file.isEmpty())
	{
		openProject(file);
	}
}

void MainWindow::exportImage()
{
	const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

	QString filename = QFileDialog::getSaveFileName(this, tr("Export Image"),
		picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last(),
		tr("Images (*.png *.xpm *.jpg)"));

	if (!filename.isEmpty())
	{
		QImageWriter writer(filename);
		if(!writer.write(ui.viewWidget->image()))
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Couldn't write image to %1:\n%2")
				.arg(filename)
				.arg(writer.errorString()));
		}
	}
}

static QString friendlyTime(quint64 ms)
{
	quint64 s = ms / 1000;
	quint64 m = s / 60;
	quint64 h = m / 60;
	quint64 d = h / 24;

	s %= 60;
	m %= 60;
	h %= 24;

	if (d >= 2 || (d == 1 && h == 0))
	{
		return QString("%1 %2").arg(d).arg(d == 1 ? "day" : "days");
	}
	else if (d == 1)
	{
		return QString("%1 day %2 %3").arg(d).arg(h).arg(h == 1 ? "hour" : "hours");
	}
	else
	{
		if (h > 0)
		{
			if (m == 0)
			{
				return QString("%1 %2").arg(h).arg(h == 1 ? "hour" : "hours");
			}
			else
			{
				return QString("%1 %2 %3 %4").arg(h).arg(h == 1 ? "hour" : "hours").arg(m).arg(m == 1 ? "min" : "mins");
			}
		}
		else
		{
			if (m > 0)
			{
				if (s == 0)
				{
					return QString("%1 %2").arg(m).arg(m == 1 ? "min" : "mins");
				}
				else
				{
					return QString("%1 %2 %3 %4").arg(m).arg(m == 1 ? "min" : "mins").arg(s).arg(s == 1 ? "sec" : "secs");
				}
			}
			else
			{
				if (s > 0)
				{
					return QString("%1 %2").arg(s).arg(s == 1 ? "sec" : "secs");
				}
				else
				{
					return QString("less then 1 sec");
				}
			}
		}
	}
}

void MainWindow::updateView()
{
	if (mRenderer)
	{
		quint64 time = mElapsedTime.elapsed();

		float percent = mRenderer->pixelsRendered() / (float)(mRenderer->width()*mRenderer->height());
		float lerp = percent*percent;

		quint64 timeLeft1 = (1 - percent) * time / PM::pm_MaxT(0.0001f, percent);
		quint64 timeLeft2 = ((1 - percent) / PM::pm_MaxT(0.0001f, (percent - mLastPercent))) * mFrameTime.elapsed();

		mLastPercent = percent;

		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Pixels: %1/%2 (%3%) | Rays: %4 | Elapsed time: %5 | Time left: %6")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(100*percent, 4)
			.arg(mRenderer->rayCount())
			.arg(friendlyTime(time))
			.arg(friendlyTime((1 - lerp)*timeLeft1 + lerp*timeLeft2)));

		setWindowTitle(tr("PearRay Viewer [ %1% ]").arg((int)percent));

		mFrameTime.restart();

		if (mRenderer->isFinished())
		{
			stopRendering();
		}
	}
	else
	{
		ui.statusBar->showMessage("No rendering");
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	writeSettings();
	event->accept();
}

void MainWindow::readSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();
}

void MainWindow::writeSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.endGroup();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About PearRay Viewer"),
		tr("<h2>About PearRay Viewer " PR_VERSION_STRING "</h2>"
		"<p>A viewer for PearRay render results.</p>"
		"<p>Author: &Ouml;mercan Yazici &lt;<a href='mailto:omercan@pearcoding.eu?subject=\"PearRay\"'>pearcoding@gmail.com</a>&gt;<br/>"
		"Copyright &copy; 2015-2016 &Ouml;mercan Yazici<br/>"
		"Website: <a href='http://pearcoding.eu/projects/pearray'>http://pearcoding.eu/projects/pearray</a></p>"
#ifdef PR_DEBUG
		"<hr /><h4>Development Information:</h4><p>"
		"Version: " PR_VERSION_STRING "<br />"
		"Compiled: " __DATE__ " " __TIME__ "<br />"
#endif
		));
}

void MainWindow::openWebsite()
{
	QDesktopServices::openUrl(QUrl("http://pearcoding.eu/projects/pearray"));
}

void MainWindow::showAllToolbars()
{
}

void MainWindow::hideAllToolbars()
{
}

void MainWindow::showAllDocks()
{
	ui.systemPropertyDockWidget->show();
	ui.spectrumDockWidget->show();
	ui.outlineDockWidget->show();
	ui.entityDetailsDockWidget->show();
}

void MainWindow::hideAllDocks()
{
	ui.systemPropertyDockWidget->hide();
	ui.spectrumDockWidget->hide();
	ui.outlineDockWidget->hide();
	ui.entityDetailsDockWidget->hide();
}

void MainWindow::startRendering()
{
	if (!mRenderer->isFinished())
	{
		return;
	}

	ui.systemPropertyView->enableRendering();
	ui.entityDetailsView->setDisabled(true);

	ui.systemPropertyView->setupRenderer(mRenderer);

	mEnvironment->scene()->buildTree();

	mTimer.start(200);
	mElapsedTime.restart();
	mFrameTime.restart();
	mRenderer->start(ui.systemPropertyView->getTileX(),
		ui.systemPropertyView->getTileY(),
		ui.systemPropertyView->getThreadCount());
}

void MainWindow::stopRendering()
{
	mTimer.stop();

	if (!mRenderer->isFinished())
	{
		mRenderer->stop();
		mRenderer->waitForFinish();
		ui.statusBar->showMessage(tr("Rendering stopped."));
	}
	else
	{
		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Pixels: %1/%2 | Rays: %3 | Render time: %4")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(mRenderer->rayCount())
			.arg(friendlyTime(mElapsedTime.elapsed())));
	}

	ui.systemPropertyView->disableRendering();
	ui.entityDetailsView->setDisabled(false);

	setWindowTitle(tr("PearRay Viewer"));
}

void MainWindow::entitySelected(QModelIndex index)
{
	if (index.isValid())
	{
		ui.entityDetailsView->setEntity((PR::Entity*)index.internalPointer());
	}
	else
	{
		ui.entityDetailsView->setEntity(nullptr);
	}
}