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

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, mRays(nullptr)
{
	ui.setupUi(this);

	connect(ui.actionOpenDump, SIGNAL(triggered()), this, SLOT(openDump()));
	connect(ui.actionOpenMultipleDumps, SIGNAL(triggered()), this, SLOT(openMultipleDumps()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	readSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::openProject(const QString& str, bool multiple)
{
	if (!mRays) {
		mRays = std::make_unique<RayArray>();
	}

	QStringList files;
	if (multiple) {
		QRegExp rx("(\\d+\\.rdmp)$");

		int pos = rx.indexIn(str);
		if (pos >= 0) {
			QString base = str;
			base.chop(str.size() - pos);

			size_t i = 0;
			while (QFile::exists(QString("%1%2.rdmp").arg(base).arg(i))) {
				files.append(QString("%1%2.rdmp").arg(base).arg(i));
				++i;
			}
		}
	}

	if (files.empty()) {
		files.append(str);
	}

	bool ok = true;
	for (QString f : files) {
		std::ifstream stream(str.toStdString());
		if (!mRays->load(stream)) {
			ok = false;
			break;
		}
	}

	if (ok) {
		statusBar()->showMessage(tr("Loaded RDMP"));

		ui.rayCountLabel->setText(QString::number(mRays->size()));

		ui.openGLWidget->clear();
		mRays->populate(ui.openGLWidget->vertices(), ui.openGLWidget->indices());
		ui.openGLWidget->rebuild();
	} else {
		statusBar()->showMessage(tr("Failed to load CNT"));
	}
}

void MainWindow::openDump()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	QString file			 = QFileDialog::getOpenFileName(this, tr("Open File"),
												docLoc.isEmpty() ? QDir::currentPath() : docLoc.last(),
												tr("Ray Dump Files (*.rdmp)"));

	if (!file.isEmpty())
		openProject(file, false);
}

void MainWindow::openMultipleDumps()
{
	const QStringList docLoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	QString file			 = QFileDialog::getOpenFileName(this, tr("Open File"),
												docLoc.isEmpty() ? QDir::currentPath() : docLoc.last(),
												tr("Ray Dump Files (*.rdmp)"));

	if (!file.isEmpty())
		openProject(file, true);
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
