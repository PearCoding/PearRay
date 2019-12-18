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
	connect(ui.toneMapperEditor, SIGNAL(changed()), this, SLOT(updateMapper()));
	connect(ui.toneMapperEditor, SIGNAL(formatChanged()), this, SLOT(updateMapperFormat()));
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

		ui.toneMapperEditor->blockSignals(true);
		ui.toneMapperEditor->setToNormal();
		updateMapperFormat();
		ui.toneMapperEditor->setToNormal();
		ui.toneMapperEditor->blockSignals(false);
		updateMapper();

		setWindowTitle(QString("[Spec] %1").arg(str));
	}
}

void SpecWindow::updateMapper()
{
	ui.imageWidget->setMapper(ui.toneMapperEditor->constructMapper());
}

void SpecWindow::updateMapperFormat()
{
	float min, max;
	ui.imageWidget->view()->getMappedMinMax(min, max, ui.toneMapperEditor->constructMapper());
	ui.toneMapperEditor->setMinMax(min, max);
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