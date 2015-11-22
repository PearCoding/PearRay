#include "MainWindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>

#include "scene/Scene.h"
#include "camera/Camera.h"
#include "renderer/Renderer.h"

#include "Environment.h"
#include "SceneLoader.h"

#include "properties/GroupProperty.h"
#include "properties/IntProperty.h"
#include "properties/ButtonProperty.h"
#include "properties/BoolProperty.h"
#include "properties/SelectionProperty.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	mEnvironment(nullptr), mRenderer(nullptr),
	mRendererGroupProp(nullptr), mRendererMaxRayDepthProp(nullptr),
	mRendererMaxDirectRayCountProp(nullptr), mRendererMaxIndirectRayCountProp(nullptr), mRendererStartProp(nullptr),
	mViewGroupProp(nullptr), mViewModeProp(nullptr)
{
	ui.setupUi(this);

	//Add tool-bars to menu

	//Add dock widgets to menu
	ui.menuDockWidgets->addAction(ui.propertyDockWidget->toggleViewAction());
	ui.menuDockWidgets->addAction(ui.spectrumDockWidget->toggleViewAction());

	// Setup properties
	mRendererGroupProp = new GroupProperty();
	mRendererGroupProp->setPropertyName(tr("Renderer"));

	mRendererTileXProp = new IntProperty();
	mRendererTileXProp->setPropertyName(tr("Tile X Count"));
	((IntProperty*)mRendererTileXProp)->setMinValue(1);
	((IntProperty*)mRendererTileXProp)->setMaxValue(128);
	((IntProperty*)mRendererTileXProp)->setDefaultValue(5);
	((IntProperty*)mRendererTileXProp)->setValue(5);
	mRendererGroupProp->addChild(mRendererTileXProp);

	mRendererTileYProp = new IntProperty();
	mRendererTileYProp->setPropertyName(tr("Tile Y Count"));
	((IntProperty*)mRendererTileYProp)->setMinValue(1);
	((IntProperty*)mRendererTileYProp)->setMaxValue(128);
	((IntProperty*)mRendererTileYProp)->setDefaultValue(5);
	((IntProperty*)mRendererTileYProp)->setValue(5);
	mRendererGroupProp->addChild(mRendererTileYProp);

	mRendererThreadsProp = new IntProperty();
	mRendererThreadsProp->setPropertyName(tr("Threads"));
	mRendererThreadsProp->setToolTip(tr("0 = Automatic"));
	((IntProperty*)mRendererThreadsProp)->setMinValue(0);
	((IntProperty*)mRendererThreadsProp)->setMaxValue(64);
	((IntProperty*)mRendererThreadsProp)->setDefaultValue(0);
	((IntProperty*)mRendererThreadsProp)->setValue(0);
	mRendererGroupProp->addChild(mRendererThreadsProp);

	mRendererSamplingProp = new BoolProperty();
	mRendererSamplingProp->setPropertyName(tr("Sampling"));
	mRendererGroupProp->addChild(mRendererSamplingProp);

	mRendererSamplesPerRayProp = new IntProperty();
	mRendererSamplesPerRayProp->setPropertyName(tr("Samples per Ray"));
	((IntProperty*)mRendererSamplesPerRayProp)->setMinValue(1);
	((IntProperty*)mRendererSamplesPerRayProp)->setMaxValue(1024);
	((IntProperty*)mRendererSamplesPerRayProp)->setDefaultValue(8);
	((IntProperty*)mRendererSamplesPerRayProp)->setValue(8);
	mRendererGroupProp->addChild(mRendererSamplesPerRayProp);

	mRendererSamplerProp = new SelectionProperty();
	mRendererSamplerProp->setPropertyName(tr("Sampler"));
	((SelectionProperty*)mRendererSamplerProp)->addItem(tr("Random"), PR::SM_Random);
	((SelectionProperty*)mRendererSamplerProp)->addItem(tr("Uniform"), PR::SM_Uniform);
	((SelectionProperty*)mRendererSamplerProp)->addItem(tr("Jitter"), PR::SM_Jitter);
	((SelectionProperty*)mRendererSamplerProp)->setDefaultIndex(2);
	((SelectionProperty*)mRendererSamplerProp)->setIndex(2);
	mRendererGroupProp->addChild(mRendererSamplerProp);

	mRendererMaxRayDepthProp = new IntProperty();
	mRendererMaxRayDepthProp->setPropertyName(tr("Max Ray Depth"));
	((IntProperty*)mRendererMaxRayDepthProp)->setMinValue(1);
	((IntProperty*)mRendererMaxRayDepthProp)->setMaxValue(128);
	((IntProperty*)mRendererMaxRayDepthProp)->setDefaultValue(2);
	((IntProperty*)mRendererMaxRayDepthProp)->setValue(2);
	mRendererGroupProp->addChild(mRendererMaxRayDepthProp);

	mRendererMaxDirectRayCountProp = new IntProperty();
	mRendererMaxDirectRayCountProp->setPropertyName(tr("Max Direct Ray Count"));
	((IntProperty*)mRendererMaxDirectRayCountProp)->setMinValue(0);
	((IntProperty*)mRendererMaxDirectRayCountProp)->setMaxValue(999999);
	((IntProperty*)mRendererMaxDirectRayCountProp)->setDefaultValue(50);
	((IntProperty*)mRendererMaxDirectRayCountProp)->setValue(50);
	mRendererGroupProp->addChild(mRendererMaxDirectRayCountProp);

	mRendererMaxIndirectRayCountProp = new IntProperty();
	mRendererMaxIndirectRayCountProp->setPropertyName(tr("Max Indirect Ray Count"));
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setMinValue(0);
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setMaxValue(999999);
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setDefaultValue(100);
	((IntProperty*)mRendererMaxIndirectRayCountProp)->setValue(100);
	mRendererGroupProp->addChild(mRendererMaxIndirectRayCountProp);

	mRendererStartProp = new ButtonProperty();
	mRendererStartProp->setPropertyName(tr("Start"));
	mRendererGroupProp->addChild(mRendererStartProp);

	mProperties.add(mRendererGroupProp);

	mViewGroupProp = new GroupProperty();
	mViewGroupProp->setPropertyName(tr("View"));
	
	mViewModeProp = new SelectionProperty();
	mViewModeProp->setPropertyName(tr("Display"));
	((SelectionProperty*)mViewModeProp)->addItem(tr("Color"), VM_Color);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Color Linear"), VM_ColorLinear);
	((SelectionProperty*)mViewModeProp)->addItem(tr("Depth"), VM_Depth);
	((SelectionProperty*)mViewModeProp)->addItem(tr("CIE XYZ"), VM_XYZ);
	((SelectionProperty*)mViewModeProp)->addItem(tr("CIE Norm XYZ"), VM_NORM_XYZ);
	((SelectionProperty*)mViewModeProp)->setDefaultIndex(0);
	mViewGroupProp->addChild(mViewModeProp);

	mProperties.add(mViewGroupProp);

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

	connect(ui.viewWidget, SIGNAL(spectrumSelected(const PR::Spectrum&)), ui.spectrumWidget, SLOT(setSpectrum(const PR::Spectrum)));

	readSettings();

	// Test env
	PRU::SceneLoader loader;
	mEnvironment = loader.loadFromFile("test.prc");

	if (mEnvironment)
	{
		mRenderer = new PR::Renderer(500, 500, mEnvironment->camera(), mEnvironment->scene());
		mRenderer->setMaxRayDepth(2);
		mRenderer->setMaxDirectRayCount(100);
		mRenderer->setMaxIndirectRayCount(100);

		ui.viewWidget->setRenderer(mRenderer);
		mEnvironment->scene()->buildTree();
	}
}

MainWindow::~MainWindow()
{
	if (mRenderer)
	{
		mRenderer->stop();
		mRenderer->waitForFinish();
		delete mRenderer;
	}

	if (mEnvironment)
	{
		delete mEnvironment;
	}

	if (mRendererGroupProp)
	{
		delete mRendererGroupProp;
		delete mRendererTileXProp;
		delete mRendererTileYProp;
		delete mRendererThreadsProp;
		delete mRendererSamplingProp;
		delete mRendererSamplesPerRayProp;
		delete mRendererSamplerProp;
		delete mRendererMaxRayDepthProp;
		delete mRendererMaxDirectRayCountProp;
		delete mRendererMaxIndirectRayCountProp;
		delete mViewGroupProp;
		delete mViewModeProp;
	}
}

void MainWindow::updateView()
{
	if (mRenderer)
	{
		float percent = 100 * mRenderer->pixelsRendered() / (float)(mRenderer->width()*mRenderer->height());
		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Pixels: %1/%2 (%3%) | Rays: %4")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(percent, 4)
			.arg(mRenderer->rayCount()));

		setWindowTitle(tr("PearRay Viewer [ %1% ]").arg((int)percent));

		if (mRenderer->isFinished())
		{
			stopRendering();
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
	else if (prop == mRendererMaxDirectRayCountProp)
	{
		mRenderer->setMaxDirectRayCount(((IntProperty*)mRendererMaxDirectRayCountProp)->value());
	}
	else if (prop == mRendererMaxIndirectRayCountProp)
	{
		mRenderer->setMaxIndirectRayCount(((IntProperty*)mRendererMaxIndirectRayCountProp)->value());
	}
	else if (prop == mRendererSamplingProp)
	{
		mRenderer->enableSampling(((BoolProperty*)mRendererSamplingProp)->value());
	}
	else if (prop == mRendererSamplesPerRayProp)
	{
		mRenderer->setSamplePerRayCount(((IntProperty*)mRendererSamplesPerRayProp)->value());
	}
	else if (prop == mRendererSamplerProp)
	{
		mRenderer->setSamplerMode((PR::SamplerMode)((SelectionProperty*)mRendererSamplerProp)->currentData().toInt());
	}
	else if (prop == mRendererStartProp)
	{
		if (mRenderer->isFinished())
		{
			startRendering();
		}
		else
		{
			stopRendering();
		}
	}
	else if (prop == mViewModeProp)
	{
		ui.viewWidget->setViewMode((ViewMode) ((SelectionProperty*)mViewModeProp)->currentData().toInt());
	}
}

void MainWindow::startRendering()
{
	if (!mRenderer->isFinished())
	{
		return;
	}

	mRendererTileXProp->setEnabled(false);
	mRendererTileYProp->setEnabled(false);
	mRendererThreadsProp->setEnabled(false);
	mRendererSamplingProp->setEnabled(false);
	mRendererSamplesPerRayProp->setEnabled(false);
	mRendererSamplerProp->setEnabled(false);
	mRendererMaxDirectRayCountProp->setEnabled(false);
	mRendererMaxIndirectRayCountProp->setEnabled(false);
	mRendererMaxRayDepthProp->setEnabled(false);
	mRendererStartProp->setPropertyName(tr("Stop"));

	mTimer.start(200);
	mRenderer->start(((IntProperty*)mRendererTileXProp)->value(),
		((IntProperty*)mRendererTileYProp)->value(),
		((IntProperty*)mRendererThreadsProp)->value());
}

void MainWindow::stopRendering()
{
	mTimer.stop();

	if (!mRenderer->isFinished())
	{
		mRenderer->stop();
		mRenderer->waitForFinish();
		ui.statusBar->showMessage(tr("Rendering stopped."));
	}
	else
	{
		ui.viewWidget->refreshView();
		ui.statusBar->showMessage(QString("Pixels: %1/%2 | Rays: %3")
			.arg(mRenderer->pixelsRendered())
			.arg(mRenderer->width()*mRenderer->height())
			.arg(mRenderer->rayCount()));
	}

	mRendererTileXProp->setEnabled(true);
	mRendererTileYProp->setEnabled(true);
	mRendererThreadsProp->setEnabled(true);
	mRendererSamplingProp->setEnabled(true);
	mRendererSamplesPerRayProp->setEnabled(true);
	mRendererSamplerProp->setEnabled(true);
	mRendererMaxDirectRayCountProp->setEnabled(true);
	mRendererMaxIndirectRayCountProp->setEnabled(true);
	mRendererMaxRayDepthProp->setEnabled(true);
	mRendererStartProp->setPropertyName(tr("Start"));

	setWindowTitle(tr("PearRay Viewer"));
}