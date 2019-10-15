#include "SpecWindow.h"
#include "io/SpecFile.h"

#include <QFileDialog>
#include <QStandardPaths>

SpecWindow::SpecWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.resetViewButton, SIGNAL(clicked()), ui.imageWidget, SLOT(resetView()));
	connect(ui.originalScaleButton, SIGNAL(clicked()), ui.imageWidget, SLOT(zoomToOriginalSize()));
	connect(ui.exportImageButton, SIGNAL(clicked()), this, SLOT(exportImage()));
}

SpecWindow::~SpecWindow()
{
}

void SpecWindow::openFile(const QString& str)
{
	std::shared_ptr<SpecFile> file = std::make_shared<SpecFile>();
	if (file->open(str)) {
		ui.sizeLabel->setText(QString("%1x%2").arg(file->width()).arg(file->height()));
		ui.imageWidget->setView(file);

		setWindowTitle(QString("[Spec] %1").arg(str));
	}
}

void SpecWindow::exportImage()
{
	const QStringList loc = QStandardPaths::standardLocations(
		QStandardPaths::PicturesLocation);

	QString file = QFileDialog::getSaveFileName(this, tr("Save Image"),
												loc.isEmpty()
													? QDir::currentPath()
													: loc.last(),
												tr("Images (*.png *.xpm *.jpg)"));

	if (!file.isEmpty()) {
		ui.imageWidget->exportImage(file);
	}
}