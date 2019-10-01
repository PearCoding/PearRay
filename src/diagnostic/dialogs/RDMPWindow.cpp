#include "RDMPWindow.h"
#include "rdmp/RayArray.h"

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

RDMPWindow::RDMPWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	mRays		   = std::make_unique<RayArray>();
	mGraphicObject = std::make_shared<GraphicObject>(true);
	ui.openGLWidget->addGraphicObject(mGraphicObject);
}

RDMPWindow::~RDMPWindow()
{
}

void RDMPWindow::openFile(const QString& str)
{
	mRays->clear();

	std::ifstream stream(str.toStdString());
	if (stream && mRays->load(stream)) {
		ui.rayCountLabel->setText(QString::number(mRays->size()));

		mRays->populate(mGraphicObject->vertices(),
						mGraphicObject->colors(),
						mGraphicObject->indices());
		ui.openGLWidget->rebuild();

		setWindowTitle(QString("[RDMP] %1").arg(str));
	} else {
		QMessageBox::critical(this, tr("Failed to load RDMP"),
							  tr("Could not load RDMP file"));
	}
}

void RDMPWindow::openDir(const QString& str)
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
		ui.rayCountLabel->setText(QString::number(mRays->size()));

		mRays->populate(mGraphicObject->vertices(),
						mGraphicObject->colors(),
						mGraphicObject->indices());
		ui.openGLWidget->rebuild();

		setWindowTitle(QString("[RDMP] %1").arg(str));
	} else {
		QMessageBox::critical(this, tr("Failed to load RDMP"),
							  tr("Could not load RDMP files from the directory"));
	}
}
