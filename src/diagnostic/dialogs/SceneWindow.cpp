#include "SceneWindow.h"
#include "3d/GraphicObject.h"
#include "cnt/Container.h"
#include "rdmp/RayArray.h"

#include <fstream>

#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDataStream>

SceneWindow::SceneWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.sceneView->addAxis();
}

SceneWindow::~SceneWindow()
{
}

void SceneWindow::clear()
{
	mCNTObjects.clear();
	mRDMPObjects.clear();

	ui.sceneView->clear();
	ui.sceneView->update();
}

void SceneWindow::openCNTFile(const QString& str)
{
	QFile file(str);

	CNTObj obj;
	obj.Container = std::make_shared<Container>();
	if (file.exists() && obj.Container->load(file)) {
		obj.Object = std::make_shared<GraphicObject>();
		obj.Container->populate(obj.Object->vertices(), obj.Object->indices(), obj.Container->depth());

		ui.sceneView->addGraphicObject(obj.Object);
		ui.sceneView->rebuild();

		mCNTObjects.append(obj);
	} else {
		QMessageBox::critical(this, tr("Failed to load CNT"), tr("Could not load the given CNT file."));
	}
}

constexpr quint32 DEFAULT_SKIP = 1;

void SceneWindow::openRDMPFile(const QString& str)
{
	RDMPObj obj;
	obj.RayArray = std::make_shared<RayArray>();

	QFile file(str);
	if (file.exists() && obj.RayArray->load(file)) {
		obj.Object = std::make_shared<GraphicObject>(true);

		obj.RayArray->populate(obj.Object->vertices(),
							   obj.Object->colors(),
							   obj.Object->indices());

		ui.sceneView->addGraphicObject(obj.Object);
		ui.sceneView->rebuild();

		mRDMPObjects.append(obj);
	} else {
		QMessageBox::critical(this, tr("Failed to load RDMP"),
							  tr("Could not load RDMP file"));
	}
}

void SceneWindow::openRDMPDir(const QString& str)
{
	RDMPObj obj;
	obj.RayArray = std::make_shared<RayArray>();

	QDir directory(str);
	QFileInfoList files = directory.entryInfoList(QStringList() << "*.rdmp",
												  QDir::Files);

	bool ok = true;
	for (QFileInfo filename : files) {
		QFile file(filename.filePath());
		if (!file.exists() || !obj.RayArray->load(file, DEFAULT_SKIP)) {
			ok = false;
			break;
		}
	}

	if (ok) {
		obj.Object = std::make_shared<GraphicObject>(true);

		obj.RayArray->populate(obj.Object->vertices(),
							   obj.Object->colors(),
							   obj.Object->indices());

		ui.sceneView->addGraphicObject(obj.Object);
		ui.sceneView->rebuild();

		mRDMPObjects.append(obj);
	} else {
		QMessageBox::critical(this, tr("Failed to load RDMP"),
							  tr("Could not load RDMP files from the directory"));
	}
}