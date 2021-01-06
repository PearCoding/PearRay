#include "MainWindow.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QThread>

#include "config/Version.h"

#include "FrameBufferView.h"
#include "Project.h"

constexpr int MAX_LAST_FILES = 10;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, mImageTimer(this)
	, mRenderingStatus(nullptr)
	, mRenderingProgress(nullptr)
	, mImageUpdateIntervalMSecs(1000)
{
	ui.setupUi(this);

	mImageTimer.setTimerType(Qt::CoarseTimer);

	// Setup status bar
	mRenderingStatus = new QLabel(this);

	// TODO: Make use of this
	mRenderingProgress = new QProgressBar(this);
	mRenderingProgress->setRange(0, 100);
	mRenderingProgress->setTextVisible(false);
	mRenderingProgress->setMaximumWidth(150 * devicePixelRatio());

	ui.statusBar->addPermanentWidget(mRenderingProgress);
	ui.statusBar->addPermanentWidget(mRenderingStatus);

	updateStatus(false);
	updateProgress(0);

	connect(ui.actionOpenFile, &QAction::triggered, this, QOverload<>::of(&MainWindow::openFile));
	connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::about);
	connect(ui.actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
	connect(ui.actionWebsite, &QAction::triggered, this, &MainWindow::openWebsite);
	connect(ui.actionQuit, &QAction::triggered, this, &MainWindow::close);

	connect(ui.actionStart, &QAction::triggered, this, &MainWindow::startStopRender);

	connect(ui.action100, &QAction::triggered, ui.imageView, &PR::UI::ImageView::zoomToOriginal);
	connect(ui.actionFit, &QAction::triggered, ui.imageView, &PR::UI::ImageView::zoomToFit);
	connect(ui.actionExport, &QAction::triggered, this, &MainWindow::exportImage);
	connect(ui.imagePipeline, &PR::UI::ImagePipelineEditor::changed, this, &MainWindow::updatePipeline);

	connect(&mImageTimer, &QTimer::timeout, this, &MainWindow::updateImage);

	setupRecentMenu();
	setupDockWidgets();
	setupToolbars();
	readSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (mProject && mProject->isRendering()) {
		if (QMessageBox::question(this, tr("Close ongoing project?"),
								  tr("Project is still rendering.\nStop rendering and close?"),
								  QMessageBox::Yes, QMessageBox::No)
			== QMessageBox::No) {
			event->ignore();
			return;
		}
		mProject->stopRendering(true);
		QThread::usleep(500);
	}
	writeSettings();
	event->accept();
}

void MainWindow::readSettings()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

	QSettings settings;

	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	mLastDir = settings.value(
						   "last_dir",
						   docLoc.isEmpty() ? QDir::homePath() : docLoc.last())
				   .toString();
	mLastFiles				  = settings.value("last_files").toStringList();
	mImageUpdateIntervalMSecs = settings.value("image_update_interval_msecs", mImageUpdateIntervalMSecs).toInt();
	ui.imageView->showUpdateRegions(settings.value("show_update_regions", true).toBool());
	settings.endGroup();

	updateRecentFiles();
}

void MainWindow::writeSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("last_dir", mLastDir);
	settings.setValue("last_files", mLastFiles);
	settings.setValue("image_update_interval_msecs", mImageUpdateIntervalMSecs);
	settings.setValue("show_update_regions", ui.imageView->isUpdateRegionsVisible());
	settings.endGroup();
}

void MainWindow::openFile()
{
	const QString file = QFileDialog::getOpenFileName(this, tr("Open File"),
													  mLastDir,
													  tr("PearRay Project (*.prc)"));

	if (!file.isEmpty()) {
		QApplication::processEvents();
		openFile(file);
	}
}

void MainWindow::openFile(const QString& file)
{
	QFileInfo info(file);

	if (!info.exists()) {
		QMessageBox::warning(this, tr("File not found"), tr("File %1 was not found").arg(file));
		return;
	}

	if (mProject) {
		if (QMessageBox::question(this, tr("Close current project?"),
								  tr("There is already a project loaded.\nClose current one?"),
								  QMessageBox::Yes, QMessageBox::No)
			== QMessageBox::No)
			return;

		closeProject();
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	try {
		mProject = new Project(file);
		connect(mProject, &Project::renderingStarted, this, &MainWindow::renderingStarted);
		connect(mProject, &Project::renderingFinished, this, &MainWindow::renderingFinished);
	} catch (const std::exception& e) {
		QMessageBox::critical(this, tr("Error while loading"), tr("Could not load file %1:\n%2").arg(file).arg(e.what()));
		QApplication::restoreOverrideCursor();
		return;
	}

	setupProjectContext();
	addToRecentFiles(file);

	QApplication::restoreOverrideCursor();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About PearRay Viewer"),
					   tr("<h2>About PearRay Viewer " PR_VERSION_STRING "</h2>"
						  "<p>A viewer for PearRay.</p>"
						  "<p>Author: &Ouml;mercan Yazici &lt;<a href='mailto:omercan@pearcoding.eu?subject=\"PearRay\"'>omercan@pearcoding.eu</a>&gt;<br/>"
						  "Copyright &copy; 2015-2020 &Ouml;mercan Yazici<br/>"
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

void MainWindow::setupDockWidgets()
{
	const std::vector<QDockWidget*> dockWidgets = {
		ui.imagePipelineDW, ui.logDW, ui.outlineDW, ui.propertiesDW
	};

	for (auto dw : dockWidgets)
		ui.menuDocks->addAction(dw->toggleViewAction());

	connect(ui.actionShow_All, &QAction::triggered, [=]() { for (auto dw : dockWidgets) dw->show(); });
	connect(ui.actionHide_All, &QAction::triggered, [=]() { for (auto dw : dockWidgets) dw->hide(); });
}

void MainWindow::setupToolbars()
{
	const std::vector<QToolBar*> toolbars = {
		ui.controlToolBar, ui.imageToolBar
	};

	for (auto t : toolbars)
		ui.menuToolbars->addAction(t->toggleViewAction());

	connect(ui.actionShow_All_2, &QAction::triggered, [=]() { for (auto t : toolbars) t->show(); });
	connect(ui.actionHide_All_2, &QAction::triggered, [=]() { for (auto t : toolbars) t->hide(); });
}

void MainWindow::setupRecentMenu()
{
	ui.menuRecentFiles->clear();
	ui.menuRecentFiles->addSection(tr("Files"));
	for (int i = 0; i < MAX_LAST_FILES; ++i) {
		QAction* action = new QAction("");
		connect(action, &QAction::triggered,
				this, &MainWindow::openRecentFile);

		ui.menuRecentFiles->addAction(action);
		mLastFileActions.append(action);
	}
}

void MainWindow::addToRecentFiles(const QString& path)
{
	const QFileInfo info(path);
	mLastDir = info.absoluteDir().path();

	if (mLastFiles.contains(path))
		mLastFiles.removeAll(path);

	mLastFiles.push_front(path);
	if (mLastFiles.size() > MAX_LAST_FILES)
		mLastFiles.pop_back();

	updateRecentFiles();
}

void MainWindow::updateRecentFiles()
{
	for (int i = 0; i < MAX_LAST_FILES; ++i) {
		if (i < mLastFiles.size()) {
			mLastFileActions[i]->setText(mLastFiles.at(i));
			mLastFileActions[i]->setVisible(true);
		} else {
			mLastFileActions[i]->setVisible(false);
		}
	}
}

void MainWindow::openRecentFile()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action)
		return;
	openFile(action->text());
}

void MainWindow::setupProjectContext()
{
	const bool enabled = mProject != nullptr;

	ui.actionStart->setEnabled(enabled);

	ui.actionFit->setEnabled(enabled);
	ui.action100->setEnabled(enabled);
	ui.actionExport->setEnabled(enabled);
}

void MainWindow::closeProject()
{
	if (!mProject)
		return;

	mProject->stopRendering();
	QThread::msleep(200);
	mProject.clear();

	setupProjectContext();
}

void MainWindow::exportImage()
{
	if (!ui.imageView->view())
		return;

	const QString file = QFileDialog::getSaveFileName(this, tr("Save Image"),
													  mLastDir,
													  tr("Images (*.png *.xpm *.jpg *.ppm)"));

	if (!file.isEmpty())
		ui.imageView->exportImage(file);
}

void MainWindow::updatePipeline()
{
	ui.imageView->setPipeline(ui.imagePipeline->constructPipeline());
}

void MainWindow::updateImage()
{
	const auto& regions = mProject->currentUpdateRegions();
	ui.imageView->setUpdateRegions(regions);
	ui.imageView->updateImage();
}

void MainWindow::startStopRender()
{
	if (!mProject)
		return;

	if (mProject->isRendering()) {
		ui.statusBar->showMessage(tr("Requested soft stop"), 10);
		mProject->stopRendering();
	} else {
		mProject->startRendering(0);
	}
}

void MainWindow::renderingStarted()
{
	ui.statusBar->showMessage(tr("Started rendering"), 10);
	updateStatus(true);

	ui.imageView->setView(std::make_shared<FrameBufferView>(mProject->frame()));
	ui.imageView->zoomToFit();
	updatePipeline();
	mImageTimer.start(mImageUpdateIntervalMSecs);
}

void MainWindow::renderingFinished()
{
	mImageTimer.stop();
	//ui.imageView->setView(nullptr);
	ui.imageView->setUpdateRegions({});

	ui.statusBar->showMessage(tr("Finished rendering"), 10);
	updateStatus(false);
}

void MainWindow::updateStatus(bool running)
{
	if (running) {
		ui.actionStart->setText(tr("Stop"));
		ui.actionStart->setIcon(QIcon(":/stop_icon"));
		mRenderingStatus->setPixmap(pixmapFromSVG(":/status_on", QSize(16, 16)));
		mRenderingStatus->setToolTip(tr("Rendering"));

		// TODO: This depends if we do progressive rendering or not
		mRenderingProgress->setRange(0, 0); // Busy
	} else {
		ui.actionStart->setText(tr("Start"));
		ui.actionStart->setIcon(QIcon(":/play_icon"));
		mRenderingStatus->setPixmap(pixmapFromSVG(":/status_off", QSize(16, 16)));
		mRenderingStatus->setToolTip(tr("Idle"));

		mRenderingProgress->setRange(0, 100);
	}
}

void MainWindow::updateProgress(float f)
{
	mRenderingProgress->setValue(100 * f);
}

QPixmap MainWindow::pixmapFromSVG(const QString& filename, const QSize& baseSize)
{
	const qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
	QIcon icon(filename);
	QPixmap pixmap = icon.pixmap(baseSize * pixelRatio);
	pixmap.setDevicePixelRatio(pixelRatio);

	return pixmap;
}