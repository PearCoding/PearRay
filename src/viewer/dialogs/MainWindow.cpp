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

	//Add tool-bars to menu

	//Add dock widgets to menu
	ui.menuDockWidgets->addAction(ui.systemPropertyDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.spectrumDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.outlineDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.entityDetailsDockWidget->toggleViewAction());

	//Connect all signal and slots
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

	// Test env
	PRU::SceneLoader loader;
	mEnvironment = loader.loadFromFile("test.prc");

	if (mEnvironment)
	{
		mRenderer = new PR::Renderer(800, 600, mEnvironment->camera(), mEnvironment->scene());
		mRenderer->setMaxRayDepth(2);
		mRenderer->setMaxDirectRayCount(100);
		mRenderer->setMaxIndirectRayCount(100);

		ui.viewWidget->setRenderer(mRenderer);

		ui.outlineView->setModel(new EntityTreeModel(mEnvironment->scene(), this));
	}
}

MainWindow::~MainWindow()
{
	if (mRenderer)
	{
		mRenderer->stop();
		mRenderer->waitForFinish();
		delete mRenderer;
	}

	if (mEnvironment)
	{
		delete mEnvironment;
	}
}

void MainWindow::exportImage()
{
	QStringList mimeTypeFilters;
	foreach(const QByteArray &mimeTypeName, QImageWriter::supportedMimeTypes())
	{
		mimeTypeFilters.append(mimeTypeName);
	}

	mimeTypeFilters.sort();

	const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

	QFileDialog dialog(this, tr("Export Image"),
		picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setMimeTypeFilters(mimeTypeFilters);
	dialog.selectMimeTypeFilter("image/png");
	dialog.setDefaultSuffix("png");

	dialog.show();
	if (dialog.exec() == QDialog::Accepted &&
		!dialog.selectedFiles().isEmpty())
	{
		QImageWriter writer(dialog.selectedFiles().first());
		if(!writer.write(ui.viewWidget->image()))
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Couldn't write image to %1:\n%2")
				.arg(dialog.selectedFiles().first())
				.arg(writer.errorString()));
		}
	}
}

void MainWindow::updateView()
{
	if (mRenderer)
	{
		float percent = 100 * mRenderer->pixelsRendered() / (float)(mRenderer->width()*mRenderer->height());
		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Pixels: %1/%2 (%3%) | Rays: %4")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(percent, 4)
			.arg(mRenderer->rayCount()));

		setWindowTitle(tr("PearRay Viewer [ %1% ]").arg((int)percent));

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
		"<p>Author: &Ouml;mercan Yazici &lt;<a href='mailto:pearcoding@gmail.com?subject=\"PearRay\"'>pearcoding@gmail.com</a>&gt;<br/>"
		"Copyright &copy; 2015-2016 PearStudios, &Ouml;mercan Yazici<br/>"
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

	mRenderer->setMaxRayDepth(ui.systemPropertyView->getMaxRayDepth());
	mRenderer->setMaxDirectRayCount(ui.systemPropertyView->getMaxDirectRayCount());
	mRenderer->setMaxIndirectRayCount(ui.systemPropertyView->getMaxIndirectRayCount());
	mRenderer->enableSampling(ui.systemPropertyView->getSampling());
	mRenderer->setSamplerMode((PR::SamplerMode)ui.systemPropertyView->getSampler());
	mRenderer->setXSampleCount(ui.systemPropertyView->getXSamples());
	mRenderer->setYSampleCount(ui.systemPropertyView->getYSamples());

	mEnvironment->scene()->buildTree();

	mTimer.start(200);
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
		ui.statusBar->showMessage(QString("Pixels: %1/%2 | Rays: %3")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(mRenderer->rayCount()));
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
		ui.entityDetailsView->setEntity(false);
	}
}