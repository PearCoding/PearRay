#include "MainWindow.h"
#include "MaterialWindow.h"

#include <fstream>

#include <QCloseEvent>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSettings>

#include "QueryEnvironment.h"
#include "config/Version.h"
#include "material/MaterialManager.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionNew_Inspection, SIGNAL(triggered()), this, SLOT(newInspection()));

	readSettings();

	mEnv = std::make_unique<PR::QueryEnvironment>(L""); // TODO: Allow parameters to change
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
	QMessageBox::about(this, tr("About PearRayMaterialLab"),
					   tr("<h2>About PearRayMaterialLab " PR_VERSION_STRING "</h2>"
						  "<p>An inspection tool for materials used by PearRay.</p>"
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

void MainWindow::newInspection()
{
	QStringList materialnames;
	const auto manager = mEnv->materialManager();

	for (auto& entry : manager->factoryMap())
		materialnames.append(QString::fromStdString(entry.first));

	materialnames.sort();

	bool ok;
	QString item = QInputDialog::getItem(this, tr("Select material"), "", materialnames, 0, false, &ok);
	if (ok && !item.isEmpty()) {
		auto factory = manager->getFactory(item.toStdString());
		if (!factory)
			return;

		MaterialWindow* w  = new MaterialWindow(item, factory.get(), ui.mdiArea);
		QMdiSubWindow* win = ui.mdiArea->addSubWindow(w);

		win->setWindowIcon(QIcon(":/image_icon"));
		w->show();
	}
}