#include "MainWindow.h"
#include "EXRWindow.h"
#include "ProfWindow.h"

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

void MainWindow::openFile(const QString& file)
{
	QFileInfo info(file);

	if (!info.exists()) {
		QMessageBox::warning(this, tr("File not found"), tr("File %1 was not found").arg(file));
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (info.suffix() == "exr") {
		EXRWindow* w	   = new EXRWindow(ui.mdiArea);
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

void MainWindow::openRecentFile()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action)
		return;
	openFile(action->text());
}