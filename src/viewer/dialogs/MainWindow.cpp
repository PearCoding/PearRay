#include "MainWindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>

#include "scene/Scene.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"

#include "entity/SphereEntity.h"

#include "material/DiffuseMaterial.h"
#include "spectral/Spectrum.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	mCamera(nullptr), mScene(nullptr), mRenderer(nullptr)
{
	ui.setupUi(this);

	//Add tool-bars to menu

	//Add dock widgets to menu
	ui.menuDockWidgets->addAction(ui.propertyDockWidget->toggleViewAction());

	//Connect all signal and slots
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	
	connect(ui.actionShowToolbars, SIGNAL(triggered()), this, SLOT(showAllToolbars()));
	connect(ui.actionHideToolbars, SIGNAL(triggered()), this, SLOT(hideAllToolbars()));
	connect(ui.actionShowDockWidgets, SIGNAL(triggered()), this, SLOT(showAllDocks()));
	connect(ui.actionHideDockWidgets, SIGNAL(triggered()), this, SLOT(hideAllDocks()));

	connect(&mTimer, SIGNAL(timeout()), this, SLOT(updateView()));
	mTimer.setSingleShot(false);
	mTimer.start(200);

	readSettings();

	// Test env
	mCamera = new PR::Camera(1, 1, 0.2f, "Camera");
	mScene = new PR::Scene("Test");
	mRenderer = new PR::Renderer(500, 500, mCamera, mScene);

	PR::Spectrum diffSpec;
	diffSpec.setValueAtWavelength(600, 1);

	PR::Spectrum emitSpec;
	emitSpec.setValueAtWavelength(600, 0.5f);
	emitSpec.setValueAtWavelength(540, 1);

	mObjectMaterial = new PR::DiffuseMaterial(diffSpec);
	mObjectMaterial->setRoughness(0.8f);

	mLightMaterial = new PR::DiffuseMaterial(diffSpec);
	mLightMaterial->setEmission(emitSpec);

	mScene->addEntity(mCamera);

	PR::SphereEntity* e = new PR::SphereEntity("Sphere 1", 2);
	e->setPosition(PM::pm_Set(4, 0, 5));
	e->setMaterial(mObjectMaterial);
	mScene->addEntity(e);

	PR::SphereEntity* e2 = new PR::SphereEntity("Sphere 2", 2);
	e2->setPosition(PM::pm_Set(-4, 0, 5));
	e2->setMaterial(mLightMaterial);
	mScene->addEntity(e2);

	ui.viewWidget->setRenderer(mRenderer);
	mScene->buildTree();
	mRenderer->render(4);
}

MainWindow::~MainWindow()
{
	if (mRenderer)
	{
		mRenderer->stop();
		mRenderer->waitForFinish();
		delete mRenderer;
	}

	if (mScene)
	{
		delete mScene;
	}

	if (mObjectMaterial)
	{
		delete mObjectMaterial;
	}

	if (mLightMaterial)
	{
		delete mLightMaterial;
	}
}

void MainWindow::updateView()
{
	if (mRenderer)
	{
		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Pixels: %1/%2 (%3%) | Rays: %4")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(100 * mRenderer->pixelsRendered() / (float)(mRenderer->width()*mRenderer->height()), 4)
			.arg(mRenderer->rayCount()));

		if (mRenderer->isFinished())
		{
			mTimer.stop();
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
		"Copyright &copy; 2015 PearStudios, &Ouml;mercan Yazici<br/>"
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
	ui.propertyDockWidget->show();
}

void MainWindow::hideAllDocks()
{
	ui.propertyDockWidget->hide();
}