#include "ImageWindow.h"
#include "io/ImageFile.h"
#include "io/ImageLayer.h"

#include <QFileDialog>
#include <QStandardPaths>

ImageWindow::ImageWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.layerList, &QListWidget::itemSelectionChanged, this, &ImageWindow::layerChanged);
	connect(ui.resetViewButton, &QToolButton::clicked, ui.imageWidget, &PR::UI::ImageView::zoomToFit);
	connect(ui.originalScaleButton, &QToolButton::clicked, ui.imageWidget, &PR::UI::ImageView::zoomToOriginal);
	connect(ui.exportImageButton, &QToolButton::clicked, this, &ImageWindow::exportImage);
	connect(ui.imagePipelineEditor, &PR::UI::ImagePipelineEditor::changed, this, &ImageWindow::updateMapper);

	mFile = std::make_unique<PR::UI::ImageFile>();
}

ImageWindow::~ImageWindow()
{
}

void ImageWindow::openFile(const QString& str)
{
	if (mFile->open(str)) {
		ui.sizeLabel->setText(QString("%1x%2").arg(mFile->width()).arg(mFile->height()));

		ui.layerList->clear();
		for (auto layer : mFile->layers())
			ui.layerList->addItem(layer->viewName());

		if (!mFile->layers().empty())
			ui.layerList->setCurrentRow(0);

		setWindowTitle(QString("[EXR] %1").arg(str));

		ui.imageWidget->zoomToOriginal();
	}
}

void ImageWindow::layerChanged()
{
	auto list = ui.layerList->selectedItems();
	if (list.empty())
		return;

	int row = ui.layerList->row(list.front());

	updateImage(row);
}

void ImageWindow::updateMapper()
{
	ui.imageWidget->setPipeline(ui.imagePipelineEditor->constructPipeline());
}

void ImageWindow::updateImage(int layerID)
{
	if (layerID >= mFile->layers().size())
		return;

	auto layer = mFile->layers()[layerID];
	ui.imageWidget->setView(layer);

	updateMapper();
}

void ImageWindow::exportImage()
{
	const QStringList loc = QStandardPaths::standardLocations(
		QStandardPaths::PicturesLocation);

	QString file = QFileDialog::getSaveFileName(this, tr("Save Image"),
												loc.isEmpty()
													? QDir::currentPath()
													: loc.last(),
												tr("Images (*.png *.xpm *.jpg *.ppm)"));

	if (!file.isEmpty()) {
		ui.imageWidget->exportImage(file);
	}
}