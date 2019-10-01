#include "CNTWindow.h"
#include "cnt/Container.h"
#include "3d/GraphicObject.h"

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

CNTWindow::CNTWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.maxDepthSlider, SIGNAL(valueChanged(int)), this, SLOT(depthChanged(int)));

	mContainer = std::make_unique<Container>();
	mGraphicObject = std::make_shared<GraphicObject>();

	ui.openGLWidget->addGraphicObject(mGraphicObject);
}

CNTWindow::~CNTWindow()
{
}

void CNTWindow::openFile(const QString& str)
{
	std::ifstream stream(str.toStdString());
	if (mContainer->load(stream)) {
		ui.nodeCountLabel->setText(QString::number(mContainer->nodeCount()));
		ui.innerNodeCountLabel->setText(QString::number(mContainer->innerNodeCount()));
		ui.maxDepthLabel->setText(QString::number(mContainer->depth()));

		ui.maxDepthSlider->setMaximum(mContainer->depth());
		ui.maxDepthSlider->setValue(mContainer->depth());

		for (int i = 0; i < 3; ++i) {
			auto itemLow = ui.tableWidget->item(0, i);
			auto itemUp  = ui.tableWidget->item(1, i);

			itemLow->setText(QString::number(mContainer->lowerBound(i)));
			itemUp->setText(QString::number(mContainer->upperBound(i)));
		}

		mContainer->populate(mGraphicObject->vertices(), mGraphicObject->indices(), mContainer->depth());
		ui.openGLWidget->rebuild();

		setWindowTitle(QString("[CNT] %1").arg(str));
	} else {
		QMessageBox::critical(this, tr("Failed to load CNT"), tr("Could not load the given CNT file."));
	}
}

void CNTWindow::depthChanged(int tick)
{
	if (mContainer) {
		mContainer->populate(mGraphicObject->vertices(), mGraphicObject->indices(), tick);
		ui.openGLWidget->rebuild();
	}
}
