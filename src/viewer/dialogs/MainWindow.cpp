#include "MainWindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QFileDialog>
#include <QImageWriter>
#include <QComboBox>

#include "scene/Scene.h"
#include "camera/Camera.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"

#include "Environment.h"
#include "SceneLoader.h"
#include "renderer/OutputMap.h"

#include "models/EntityTreeModel.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	mRenderFactory(nullptr)
{
	ui.setupUi(this);

	closeProject();// Initial

	//Add tool-bars to menu
	ui.menuToolbars->addAction(ui.mainToolBar->toggleViewAction());

	//Add dock widgets to menu
	ui.menuDockWidgets->addAction(ui.systemPropertyDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.spectrumDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.outlineDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.entityDetailsDockWidget->toggleViewAction());

	// Add view combobox
	QComboBox* viewColorCombo = new QComboBox(this);
	viewColorCombo->addItem(tr("sRGB"));
	viewColorCombo->addItem(tr("CIE XYZ"));
	viewColorCombo->addItem(tr("CIE norm XYZ"));
	viewColorCombo->addItem(tr("Luminance"));
	viewColorCombo->setToolTip(tr("Color Mode"));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addWidget(viewColorCombo);
	connect(viewColorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setViewColorMode(int)));

	QComboBox* viewGammaCombo = new QComboBox(this);
	viewGammaCombo->addItem(tr("None"));
	viewGammaCombo->addItem(tr("2.2 sRGB"));
	viewGammaCombo->setCurrentIndex(1);
	viewGammaCombo->setToolTip(tr("Gamma Mode"));
	ui.mainToolBar->addWidget(viewGammaCombo);
	connect(viewGammaCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setViewGammaMode(int)));

	QComboBox* viewMapperCombo = new QComboBox(this);
	viewMapperCombo->addItem(tr("None"));
	viewMapperCombo->addItem(tr("Reinhard"));
	viewMapperCombo->addItem(tr("Clamp"));
	viewMapperCombo->addItem(tr("Absolute"));
	viewMapperCombo->addItem(tr("Positive"));
	viewMapperCombo->addItem(tr("Negative"));
	viewMapperCombo->addItem(tr("Spherical"));
	viewMapperCombo->addItem(tr("Normalized"));
	viewMapperCombo->setCurrentIndex(1);
	viewMapperCombo->setToolTip(tr("Tone Mapper"));
	ui.mainToolBar->addWidget(viewMapperCombo);
	connect(viewMapperCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setViewMapperMode(int)));

	QComboBox* viewDisplayCombo = new QComboBox(this);
	viewDisplayCombo->addItem(tr("RGB"));
	viewDisplayCombo->addItem(tr("P"));
	viewDisplayCombo->addItem(tr("N"));
	viewDisplayCombo->addItem(tr("Ng"));
	viewDisplayCombo->addItem(tr("Nx"));
	viewDisplayCombo->addItem(tr("Ny"));
	viewDisplayCombo->addItem(tr("V"));
	viewDisplayCombo->addItem(tr("UV"));
	viewDisplayCombo->addItem(tr("dPdU"));
	viewDisplayCombo->addItem(tr("dPdV"));
	viewDisplayCombo->addItem(tr("dPdW"));
	viewDisplayCombo->addItem(tr("dPdX"));
	viewDisplayCombo->addItem(tr("dPdY"));
	viewDisplayCombo->addItem(tr("dPdZ"));
	viewDisplayCombo->addItem(tr("dPdT"));
	viewDisplayCombo->addItem(tr("D"));
	viewDisplayCombo->addItem(tr("T"));
	viewDisplayCombo->addItem(tr("Q"));
	viewDisplayCombo->addItem(tr("Mat"));
	viewDisplayCombo->addItem(tr("ID"));
	viewDisplayCombo->addItem(tr("S"));
	viewDisplayCombo->setCurrentIndex(0);
	viewDisplayCombo->setToolTip(tr("Channel"));
	ui.mainToolBar->addWidget(viewDisplayCombo);
	connect(viewDisplayCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setViewDisplayMode(int)));

	//Connect all signal and slots
	connect(ui.actionOpenScene, SIGNAL(triggered()), this, SLOT(openScene()));
	connect(ui.actionExportImage, SIGNAL(triggered()), this, SLOT(exportImage()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.actionStartRender, SIGNAL(triggered()), this, SLOT(startRendering()));
	connect(ui.actionRestartRender, SIGNAL(triggered()), this, SLOT(restartRendering()));
	connect(ui.actionCancelRender, SIGNAL(triggered()), this, SLOT(stopRendering()));
	
	connect(ui.actionShowToolbars, SIGNAL(triggered()), this, SLOT(showAllToolbars()));
	connect(ui.actionHideToolbars, SIGNAL(triggered()), this, SLOT(hideAllToolbars()));
	connect(ui.actionShowDockWidgets, SIGNAL(triggered()), this, SLOT(showAllDocks()));
	connect(ui.actionHideDockWidgets, SIGNAL(triggered()), this, SLOT(hideAllDocks()));

	connect(ui.actionReset_Zoom, SIGNAL(triggered()), ui.viewWidget, SLOT(resetZoomPan()));
	connect(ui.actionFit_Image_into_Area, SIGNAL(triggered()), ui.viewWidget, SLOT(fitIntoWindow()));
	connect(ui.actionZoom_In, SIGNAL(triggered()), ui.viewWidget, SLOT(zoomIn()));
	connect(ui.actionZoom_Out, SIGNAL(triggered()), ui.viewWidget, SLOT(zoomOut()));

	connect(ui.actionSelection_Tool, SIGNAL(triggered(bool)), this, SLOT(selectSelectionTool(bool)));
	connect(ui.actionPan_Tool, SIGNAL(triggered(bool)), this, SLOT(selectPanTool(bool)));
	connect(ui.actionZoom_Tool, SIGNAL(triggered(bool)), this, SLOT(selectZoomTool(bool)));
	connect(ui.actionCrop_Tool, SIGNAL(triggered(bool)), this, SLOT(selectCropTool(bool)));

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

	if (mRenderContext)
	{
		mRenderContext->stop();
		mRenderContext->waitForFinish();
		mRenderContext.reset();
	}

	if (mRenderFactory)
	{
		delete mRenderFactory;
		mRenderFactory = nullptr;
	}

	if (mEnvironment)
		mEnvironment.reset();
}

void MainWindow::openProject(const QString& str)
{
	if (mEnvironment)
		closeProject();

	mEnvironment = PR::SceneLoader::loadFromFile(str.toStdString());

	if (mEnvironment)
	{
		if(!mEnvironment->scene().activeCamera())
		{
			QMessageBox::warning(this, tr("No camera"), tr("No camera was set in given scene."));
			mEnvironment.reset();
			return;
		}
		
		mRenderFactory = new PR::RenderFactory(mEnvironment->renderWidth(), mEnvironment->renderHeight(),
			mEnvironment->scene(),
			QDir::tempPath().toStdString());
		mRenderFactory->settings().setCropMaxX(mEnvironment->cropMaxX());
		mRenderFactory->settings().setCropMinX(mEnvironment->cropMinX());
		mRenderFactory->settings().setCropMaxY(mEnvironment->cropMaxY());
		mRenderFactory->settings().setCropMinY(mEnvironment->cropMinY());

		ui.systemPropertyView->fillContent(mRenderFactory);
		ui.outlineView->setModel(new EntityTreeModel(mEnvironment->scene(), this));
		ui.outlineView->setEnabled(true);
		ui.viewWidget->setEnabled(true);
		ui.entityDetailsView->setEnabled(true);
		ui.systemPropertyView->setEnabled(true);

		ui.viewWidget->fitIntoWindow();
	}
	else
	{
		QMessageBox::warning(this, tr("Couldn't load project."), tr("Couldn't load project file:\n%1").arg(str));
	}
}

void MainWindow::openScene()
{
	if (mEnvironment && mRenderContext && !mRenderContext->isFinished())
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
		openProject(file);
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

inline QString friendlyHugeNumber(quint64 nm)
{
	const QLocale& def = QLocale::c();

	QString s = QString::number(nm);
	QString output;
	int i;

	for (i = s.length(); i > 3; i -= 3)
	{
		output = def.groupSeparator() + s.mid(i - 3, 3) + output;
	}

	if (i > 0)
		output = s.mid(0, i) + output;

	return output;
}

void MainWindow::updateView()
{
	if (mRenderContext)
	{
		quint64 time = mElapsedTime.elapsed();

		PR::RenderStatus status = mRenderContext->status();

		quint64 timeLeft = (1 - status.percentage()) * time / std::max(0.0001f, status.percentage());

		mLastPercent = status.percentage();

		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(
		QString("%1% | Pass: %2 | Samples: %3 | Rays: %4 | Entity Hits: %5 | Background Hits: %6 | Elapsed time: %7 | Time left: %8")
			.arg(100*status.percentage(), 4)
			.arg(mRenderContext->currentPass() + 1)
			.arg(friendlyHugeNumber(status.getField("global.pixel_sample_count").getUInt()))
			.arg(friendlyHugeNumber(status.getField("global.ray_count").getUInt()))
			.arg(friendlyHugeNumber(status.getField("global.entity_hit_count").getUInt()))
			.arg(friendlyHugeNumber(status.getField("global.background_hit_count").getUInt()))
			.arg(friendlyTime(time))
			.arg(friendlyTime(timeLeft)));

		setWindowTitle(tr("PearRay Viewer [ %1% ]").arg((int)(status.percentage()*100)));

		mFrameTime.restart();

		if (mRenderContext->isFinished())
			stopRendering();
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
		"<p>Author: &Ouml;mercan Yazici &lt;<a href='mailto:omercan@pearcoding.eu?subject=\"PearRay\"'>omercan@pearcoding.eu</a>&gt;<br/>"
		"Copyright &copy; 2015-2016 &Ouml;mercan Yazici<br/>"
		"Website: <a href='http://pearcoding.eu/projects/pearray'>http://pearcoding.eu/projects/pearray</a></p>"
			"<hr /><p>Icon Set: <a href='https://design.google.com/icons/'>https://design.google.com/icons/</a></p>"
#ifdef PR_DEBUG
		"<hr /><h4>Development Information:</h4><p>"
		"Version: " PR_VERSION_STRING "<br />"
		"Compiled: " __DATE__ " " __TIME__ "<br /></p>"
#endif
		));
}

void MainWindow::openWebsite()
{
	QDesktopServices::openUrl(QUrl("http://pearcoding.eu/projects/pearray"));
}

void MainWindow::showAllToolbars()
{
	ui.mainToolBar->show();
}

void MainWindow::hideAllToolbars()
{
	ui.mainToolBar->hide();
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
	startRendering(true);
}

void MainWindow::restartRendering()
{
	startRendering(false);
}

void MainWindow::startRendering(bool clear)
{
	if(!mRenderFactory)
		return;

	if (mRenderContext)
		return;

	QRect cropRect = ui.viewWidget->selectedCropRect();
	if (cropRect.isValid() && cropRect.width() > 2 && cropRect.height() > 2)
	{
		float xmin = qMin(qMax(cropRect.left() / (float)mRenderFactory->fullWidth(), 0.0f), 1.0f);
		float xmax = qMin(qMax(cropRect.right() / (float)mRenderFactory->fullWidth(), 0.0f), 1.0f);
		float ymin = qMin(qMax(cropRect.top() / (float)mRenderFactory->fullHeight(), 0.0f), 1.0f);
		float ymax = qMin(qMax(cropRect.bottom() / (float)mRenderFactory->fullHeight(), 0.0f), 1.0f);
		mRenderFactory->settings().setCropMaxX(xmax);
		mRenderFactory->settings().setCropMinX(xmin);
		mRenderFactory->settings().setCropMaxY(ymax);
		mRenderFactory->settings().setCropMinY(ymin);
	}
	else
	{
		mRenderFactory->settings().setCropMaxX(1);
		mRenderFactory->settings().setCropMinX(0);
		mRenderFactory->settings().setCropMaxY(1);
		mRenderFactory->settings().setCropMinY(0);
	}

	// Setup context 

	ui.systemPropertyView->setupRenderer(mRenderFactory);
	mRenderContext = mRenderFactory->create();
	ui.viewWidget->setCropSelection(
			QPoint(mEnvironment->cropMinX()*mRenderFactory->fullWidth(), mEnvironment->cropMinY()*mRenderFactory->fullHeight()),
			QPoint(mEnvironment->cropMaxX()*mRenderFactory->fullWidth(), mEnvironment->cropMaxY()*mRenderFactory->fullHeight()));
			
	ui.viewWidget->setRenderer(mRenderContext.get());

	// Setup controls
	ui.actionStartRender->setEnabled(false);
	ui.actionRestartRender->setEnabled(false);
	ui.actionCancelRender->setEnabled(true);

	ui.systemPropertyView->setEnabled(false);
	ui.entityDetailsView->setEnabled(false);

	mEnvironment->scene().freeze();
	mEnvironment->scene().buildTree();
	mEnvironment->scene().setup(mRenderContext);

	mEnvironment->outputSpecification().setup(mRenderContext);

	// if(clear)
	// 	mDisplayBuffer->clear(0,0,0,0);

	if(mRenderContext->gpu())
		mTimer.start(1000);
	else
		mTimer.start(4000);
	
	mElapsedTime.restart();
	mFrameTime.restart();
	mRenderContext->start(ui.systemPropertyView->getTileX(),
		ui.systemPropertyView->getTileY(),
		ui.systemPropertyView->getThreadCount());
}

void MainWindow::stopRendering()
{
	mTimer.stop();

	if (!mRenderContext)
		return;
	
	if (!mRenderContext->isFinished())
	{
		mRenderContext->stop();
		mRenderContext->waitForFinish();
		ui.statusBar->showMessage(tr("Rendering stopped."));
	}
	else
	{
		PR::RenderStatus status = mRenderContext->status();

		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Samples: %1 | Rays: %2 | Entity Hits: %3 | Background Hits: %4 | Render time: %5")
			.arg(friendlyHugeNumber(status.getField("global.pixel_sample_count").getUInt()))
			.arg(friendlyHugeNumber(status.getField("global.ray_count").getUInt()))
			.arg(friendlyHugeNumber(status.getField("global.entity_hit_count").getUInt()))
			.arg(friendlyHugeNumber(status.getField("global.background_hit_count").getUInt()))
			.arg(friendlyTime(mElapsedTime.elapsed())));
	}

	mRenderContext.reset();

	ui.actionStartRender->setEnabled(true);
	ui.actionRestartRender->setEnabled(true);
	ui.actionCancelRender->setEnabled(false);

	ui.systemPropertyView->setEnabled(true);
	ui.entityDetailsView->setEnabled(true);

	ui.viewWidget->repaint();

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

void MainWindow::selectSelectionTool(bool b)
{
	ui.actionSelection_Tool->setChecked(true);
	ui.actionPan_Tool->setChecked(false);
	ui.actionZoom_Tool->setChecked(false);
	ui.actionCrop_Tool->setChecked(false);
	ui.viewWidget->setToolMode(TM_Selection);
}

void MainWindow::selectPanTool(bool b)
{
	ui.actionSelection_Tool->setChecked(false);
	ui.actionPan_Tool->setChecked(true);
	ui.actionZoom_Tool->setChecked(false);
	ui.actionCrop_Tool->setChecked(false);
	ui.viewWidget->setToolMode(TM_Pan);
}

void MainWindow::selectZoomTool(bool b)
{
	ui.actionSelection_Tool->setChecked(false);
	ui.actionPan_Tool->setChecked(false);
	ui.actionZoom_Tool->setChecked(true);
	ui.actionCrop_Tool->setChecked(false);
	ui.viewWidget->setToolMode(TM_Zoom);
}

void MainWindow::selectCropTool(bool b)
{
	ui.actionSelection_Tool->setChecked(false);
	ui.actionPan_Tool->setChecked(false);
	ui.actionZoom_Tool->setChecked(false);
	ui.actionCrop_Tool->setChecked(true);
	ui.viewWidget->setToolMode(TM_Crop);
}

void MainWindow::setViewColorMode(int i)
{
	ui.viewWidget->setColorMode((PR::ToneColorMode)i);
}

void MainWindow::setViewGammaMode(int i)
{
	ui.viewWidget->setGammaMode((PR::ToneGammaMode)i);
}

void MainWindow::setViewMapperMode(int i)
{
	ui.viewWidget->setMapperMode((PR::ToneMapperMode)i);
}

void MainWindow::setViewDisplayMode(int i)
{
	ui.viewWidget->setDisplayMode(i);
}
