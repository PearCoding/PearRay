#include "MainWindow.h"
#include "RayArray.h"

#include <fstream>

#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QSettings>

// We do not link to the library, only include the configuration file!
#include "PR_Config.h"

constexpr quint32 DEFAULT_SKIP = 1;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionOpenDump, SIGNAL(triggered()), this, SLOT(openDump()));
	connect(ui.actionOpenMultipleDumps, SIGNAL(triggered()), this, SLOT(openMultipleDumps()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	readSettings();

	mRays		   = std::make_unique<RayArray>();
	mGraphicObject = std::make_shared<GraphicObject>();
	ui.openGLWidget->addGraphicObject(mGraphicObject);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openProject(const QString& str)
{
	mRays->clear();

	std::ifstream stream(str.toStdString());
	if (stream && mRays->load(stream)) {
		statusBar()->showMessage(tr("Loaded RDMP"));

		ui.rayCountLabel->setText(QString::number(mRays->size()));

		mRays->populate(mGraphicObject->vertices(), mGraphicObject->indices());
		ui.openGLWidget->rebuild();
	} else {
		statusBar()->showMessage(tr("Failed to load RDMP"));
	}
}

void MainWindow::openProjectDir(const QString& str)
{
	mRays->clear();

	QDir directory(str);
	QFileInfoList files = directory.entryInfoList(QStringList() << "*.rdmp",
											QDir::Files);

	bool ok = true;
	for (QFileInfo filename : files) {
		//qDebug() << filename.filePath();
		std::ifstream stream(filename.filePath().toStdString());
		if (!stream || !mRays->load(stream, DEFAULT_SKIP)) {
			ok = false;
			break;
		}
	}

	if (ok) {
		statusBar()->showMessage(tr("Loaded RDMP"));

		ui.rayCountLabel->setText(QString::number(mRays->size()));

		mRays->populate(mGraphicObject->vertices(), mGraphicObject->indices());
		ui.openGLWidget->rebuild();
	} else {
		statusBar()->showMessage(tr("Failed to load RDMPs from directory"));
	}
}

void MainWindow::openDump()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

	if (mLastDir.isEmpty()) {
		mLastDir = docLoc.isEmpty() ? QDir::currentPath() : docLoc.last();
	}

	QString file = QFileDialog::getOpenFileName(this, tr("Open File"),
												mLastDir,
												tr("Ray Dump Files (*.rdmp)"));

	if (!file.isEmpty()) {
		openProject(file);
		mLastDir = QFileInfo(file).dir().path();
	}
}

void MainWindow::openMultipleDumps()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

	if (mLastDir.isEmpty()) {
		mLastDir = docLoc.isEmpty() ? QDir::currentPath() : docLoc.last();
	}

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
													mLastDir);

	if (!dir.isEmpty()) {
		openProjectDir(dir);
		QDir d2 = QDir(dir);
		if (d2.cdUp()) {
			mLastDir = d2.path();
		}
	}
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

void MainWindow::about()
{
	QMessageBox::about(this, tr("About PearRay RDMP Viewer"),
					   tr("<h2>About PearRay RDMP Viewer " PR_VERSION_STRING "</h2>"
						  "<p>A viewer for PearRay RDMP files dumped by the internal ray buffers.</p>"
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
