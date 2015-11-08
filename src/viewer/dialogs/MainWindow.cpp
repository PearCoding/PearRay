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

#include "properties/GroupProperty.h"
#include "properties/IntProperty.h"
#include "properties/ButtonProperty.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	mCamera(nullptr), mScene(nullptr), mRenderer(nullptr),
	mRendererGroupProp(nullptr), mRendererMaxRayDepthProp(nullptr),
	mRendererMaxBounceRayCountProp(nullptr), mRendererStartProp(nullptr)
{
	ui.setupUi(this);

	//Add tool-bars to menu

	//Add dock widgets to menu
	ui.menuDockWidgets->addAction(ui.propertyDockWidget->toggleViewAction());

	// Setup properties
	mRendererGroupProp = new GroupProperty();
	mRendererGroupProp->setPropertyName(tr("Renderer"));

	mRendererMaxRayDepthProp = new IntProperty();
	mRendererMaxRayDepthProp->setPropertyName(tr("Max Ray Depth"));
	((IntProperty*)mRendererMaxRayDepthProp)->setMinValue(1);
	((IntProperty*)mRendererMaxRayDepthProp)->setMaxValue(128);
	((IntProperty*)mRendererMaxRayDepthProp)->setDefaultValue(2);
	((IntProperty*)mRendererMaxRayDepthProp)->setValue(2);
	mRendererGroupProp->addChild(mRendererMaxRayDepthProp);

	mRendererMaxBounceRayCountProp = new IntProperty();
	mRendererMaxBounceRayCountProp->setPropertyName(tr("Max Bounce Ray Count"));
	((IntProperty*)mRendererMaxBounceRayCountProp)->setMinValue(1);
	((IntProperty*)mRendererMaxBounceRayCountProp)->setMaxValue(999999);
	((IntProperty*)mRendererMaxBounceRayCountProp)->setDefaultValue(100);
	((IntProperty*)mRendererMaxBounceRayCountProp)->setValue(100);
	mRendererGroupProp->addChild(mRendererMaxBounceRayCountProp);

	mRendererStartProp = new ButtonProperty();
	mRendererStartProp->setPropertyName(tr("Start"));
	mRendererGroupProp->addChild(mRendererStartProp);

	mProperties.add(mRendererGroupProp);
	ui.propertyView->setPropertyTable(&mProperties);
	ui.propertyView->expandToDepth(1);

	//Connect all signal and slots
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionWebsite, SIGNAL(triggered()), this, SLOT(openWebsite()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	
	connect(ui.actionShowToolbars, SIGNAL(triggered()), this, SLOT(showAllToolbars()));
	connect(ui.actionHideToolbars, SIGNAL(triggered()), this, SLOT(hideAllToolbars()));
	connect(ui.actionShowDockWidgets, SIGNAL(triggered()), this, SLOT(showAllDocks()));
	connect(ui.actionHideDockWidgets, SIGNAL(triggered()), this, SLOT(hideAllDocks()));

	connect(&mProperties, SIGNAL(valueChanged(IProperty*)), this, SLOT(propertyValueChanged(IProperty*)));

	connect(&mTimer, SIGNAL(timeout()), this, SLOT(updateView()));
	mTimer.setSingleShot(false);

	readSettings();

	// Test env
	mCamera = new PR::Camera(1, 1, 0.2f, "Camera");
	mScene = new PR::Scene("Test");
	mRenderer = new PR::Renderer(500, 500, mCamera, mScene);
	mRenderer->setMaxRayDepth(2);
	mRenderer->setMaxRayBounceCount(100);
	//mRenderer->enableSubPixels(true);

	PR::Spectrum diffSpec;
	diffSpec += 0.1f;
	diffSpec.setValueAtWavelength(600, 1);

	PR::Spectrum emitSpec;
	emitSpec.setValueAtWavelength(600, 4.0f);
	//emitSpec.setValueAtWavelength(540, 0.5f);

	mObjectMaterial = new PR::DiffuseMaterial(diffSpec);
	mObjectMaterial->setRoughness(0.5f);

	mLightMaterial = new PR::DiffuseMaterial(diffSpec);
	mLightMaterial->setEmission(emitSpec);
	mLightMaterial->enableShading(false);// Only emission

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

	if (mRendererGroupProp)
	{
		delete mRendererGroupProp;
		delete mRendererMaxRayDepthProp;
		delete mRendererMaxBounceRayCountProp;
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
			
			mRendererMaxBounceRayCountProp->setEnabled(true);
			mRendererMaxRayDepthProp->setEnabled(true);
			mRendererStartProp->setPropertyName(tr("Start"));
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

void MainWindow::propertyValueChanged(IProperty* prop)
{
	if (prop == mRendererMaxRayDepthProp)
	{
		mRenderer->setMaxRayDepth(((IntProperty*)mRendererMaxRayDepthProp)->value());
	}
	else if (prop == mRendererMaxRayDepthProp)
	{
		mRenderer->setMaxRayBounceCount(((IntProperty*)mRendererMaxBounceRayCountProp)->value());
	}
	else if (prop == mRendererStartProp)
	{
		if (mRenderer->isFinished())
		{
			mRendererMaxBounceRayCountProp->setEnabled(false);
			mRendererMaxRayDepthProp->setEnabled(false);
			mRendererStartProp->setPropertyName(tr("Stop"));

			mTimer.start(200);
			mRenderer->start(10, 10);
		}
		else
		{
			mTimer.stop();

			mRenderer->stop();
			mRenderer->waitForFinish();

			ui.statusBar->showMessage(tr("Rendering stopped."));

			mRendererMaxBounceRayCountProp->setEnabled(true);
			mRendererMaxRayDepthProp->setEnabled(true);
			mRendererStartProp->setPropertyName(tr("Start"));
		}
	}
}