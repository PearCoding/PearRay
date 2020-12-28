#include "EXRWindow.h"
#include "io/EXRFile.h"

#include <QFileDialog>
#include <QStandardPaths>

EXRWindow::EXRWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.layerList, &QListWidget::itemSelectionChanged, this, &EXRWindow::layerChanged);
	connect(ui.resetViewButton, &QToolButton::clicked, ui.imageWidget, &PR::UI::ImageView::zoomToFit);
	connect(ui.originalScaleButton, &QToolButton::clicked, ui.imageWidget, &PR::UI::ImageView::zoomToOriginal);
	connect(ui.exportImageButton, &QToolButton::clicked, this, &EXRWindow::exportImage);
	connect(ui.imagePipelineEditor, &PR::UI::ImagePipelineEditor::changed, this, &EXRWindow::updateMapper);

	mFile = std::make_unique<PR::UI::EXRFile>();
}

EXRWindow::~EXRWindow()
{
}

void EXRWindow::openFile(const QString& str)
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

void EXRWindow::layerChanged()
{
	auto list = ui.layerList->selectedItems();
	if (list.empty())
		return;

	int row = ui.layerList->row(list.front());

	updateImage(row);
}

void EXRWindow::updateMapper()
{
	ui.imageWidget->setPipeline(ui.imagePipelineEditor->constructPipeline());
}

void EXRWindow::updateImage(int layerID)
{
	if (layerID >= mFile->layers().size())
		return;

	auto layer = mFile->layers()[layerID];
	ui.imageWidget->setView(layer);

	updateMapper();
}

void EXRWindow::exportImage()
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