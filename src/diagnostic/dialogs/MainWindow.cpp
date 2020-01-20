#include "MainWindow.h"
#include "EXRWindow.h"
#include "ProfWindow.h"
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
#include "config/Version.h"

constexpr int MAX_LAST_FILES = 10;

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

	setupRecentMenu();
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
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

	QSettings settings;

	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	mLastDir = settings.value(
						   "last_dir",
						   docLoc.isEmpty() ? QDir::homePath() : docLoc.last())
				   .toString();
	mLastFiles = settings.value("last_files").toStringList();
	mLastDirs  = settings.value("last_dirs").toStringList();
	settings.endGroup();

	updateRecentFiles();
	updateRecentDirs();
}

void MainWindow::writeSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("last_dir", mLastDir);
	settings.setValue("last_files", mLastFiles);
	settings.setValue("last_dirs", mLastDirs);
	settings.endGroup();
}

void MainWindow::openFile()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open File"),
												mLastDir,
												tr("Supported Files (*.cnt *.rdmp *.exr *.spec *.prof);;CNT Files (*.cnt);;RDMP Files (*.rdmp);;EXR Files (*.exr);;Spectral Files (*.spec);;Profile Files (*.prof)"));

	if (!file.isEmpty()) {
		QApplication::processEvents();
		openFile(file);
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
		QApplication::processEvents();
		openRDMPDir(dir);

		QDir d2 = QDir(dir);
		if (d2.cdUp()) {
			mLastDir = d2.path();
		}
	}
}

void MainWindow::openFile(const QString& file)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
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
	} else if (info.suffix() == "prof") {
		ProfWindow* w	  = new ProfWindow(ui.mdiArea);
		QMdiSubWindow* win = ui.mdiArea->addSubWindow(w);

		win->setWindowIcon(QIcon(":/timeline_icon"));
		w->show();
		w->openFile(file);
	}

	addToRecentFiles(file);

	QApplication::restoreOverrideCursor();
}

void MainWindow::openRDMPDir(const QString& dir)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	setupSceneWindow();
	mCurrentSceneWindow->openRDMPDir(dir);
	addToRecentDirs(dir);
	QApplication::restoreOverrideCursor();
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
	ui.menuRecentFiles->addSection(tr("Directories"));
	for (int i = 0; i < MAX_LAST_FILES; ++i) {
		QAction* action = new QAction("");
		connect(action, &QAction::triggered,
				this, &MainWindow::openRecentDir);

		ui.menuRecentFiles->addAction(action);
		mLastDirActions.append(action);
	}
}

void MainWindow::addToRecentFiles(const QString& path)
{
	mLastDir = QFileInfo(path).dir().path();

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

void MainWindow::addToRecentDirs(const QString& path)
{
	mLastDir = QFileInfo(path).dir().path();

	if (mLastDirs.contains(path))
		mLastDirs.removeAll(path);

	mLastDirs.push_front(path);
	if (mLastDirs.size() > MAX_LAST_FILES)
		mLastDirs.pop_back();

	updateRecentDirs();
}

void MainWindow::updateRecentDirs()
{
	for (int i = 0; i < MAX_LAST_FILES; ++i) {
		if (i < mLastDirs.size()) {
			mLastDirActions[i]->setText(mLastDirs.at(i));
			mLastDirActions[i]->setVisible(true);
		} else {
			mLastDirActions[i]->setVisible(false);
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

void MainWindow::openRecentDir()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action)
		return;
	openRDMPDir(action->text());
}