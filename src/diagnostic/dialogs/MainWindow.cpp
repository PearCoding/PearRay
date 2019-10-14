#include "MainWindow.h"
#include "EXRWindow.h"
#include "SceneWindow.h"
#include "SpecWindow.h"

#include <fstream>

#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QImageWriter>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSettings>

// We do not link to the library, only include the configuration file!
#include "PR_Config.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, mCurrentSceneWindow(nullptr)
{
	ui.setupUi(this);

	connect(ui.actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(ui.actionOpenRDMPDir, SIGNAL(triggered()), this, SLOT(openRDMPDir()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	readSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* event)
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
	mLastDir = settings.value("last_dir").toString();
	settings.endGroup();
}

void MainWindow::writeSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("last_dir", mLastDir);
	settings.endGroup();
}

void MainWindow::openFile()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

	if (mLastDir.isEmpty()) {
		mLastDir = docLoc.isEmpty() ? QDir::currentPath() : docLoc.last();
	}

	QString file = QFileDialog::getOpenFileName(this, tr("Open File"),
												mLastDir,
												tr("Supported Files (*.cnt *.rdmp *.exr *.spec);;CNT Files (*.cnt);;RDMP Files (*.rdmp);;EXR Files (*.exr);;Spectral Files (*.spec)"));

	if (!file.isEmpty()) {
		openFile(file);
		mLastDir = QFileInfo(file).dir().path();
	}
}

void MainWindow::openRDMPDir()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

	if (mLastDir.isEmpty()) {
		mLastDir = docLoc.isEmpty() ? QDir::currentPath() : docLoc.last();
	}

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open RDMP Directory"),
													mLastDir);

	if (!dir.isEmpty()) {
		openRDMPDir(dir);

		QDir d2 = QDir(dir);
		if (d2.cdUp()) {
			mLastDir = d2.path();
		}
	}
}

void MainWindow::openFile(const QString& file)
{
	QFileInfo info(file);

	if (info.suffix() == "cnt") {
		setupSceneWindow();
		mCurrentSceneWindow->openCNTFile(file);
	} else if (info.suffix() == "rdmp") {
		setupSceneWindow();
		mCurrentSceneWindow->openRDMPFile(file);
	} else if (info.suffix() == "exr") {
		EXRWindow* w	   = new EXRWindow(ui.mdiArea);
		QMdiSubWindow* win = ui.mdiArea->addSubWindow(w);

		win->setWindowIcon(QIcon(":/image_icon"));
		w->show();
		w->openFile(file);
	} else if (info.suffix() == "spec") {
		SpecWindow* w	  = new SpecWindow(ui.mdiArea);
		QMdiSubWindow* win = ui.mdiArea->addSubWindow(w);

		win->setWindowIcon(QIcon(":/image_icon"));
		w->show();
		w->openFile(file);
	}
}

void MainWindow::openRDMPDir(const QString& dir)
{
	setupSceneWindow();
	mCurrentSceneWindow->openRDMPDir(dir);
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About PearRayDiagnostic"),
					   tr("<h2>About PearRayDiagnostic " PR_VERSION_STRING "</h2>"
						  "<p>A diagnostic tool for files produced by PearRay.</p>"
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

void MainWindow::setupSceneWindow()
{
	if (!mCurrentSceneWindow) {
		mCurrentSceneWindow = new SceneWindow(ui.mdiArea);
		QMdiSubWindow* win  = ui.mdiArea->addSubWindow(mCurrentSceneWindow);

		win->setWindowIcon(QIcon(":/scene_icon"));
		mCurrentSceneWindow->show();
	}
}