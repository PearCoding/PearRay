#include "EXRWindow.h"
#include "exr/EXRFile.h"

#include <QFileDialog>
#include <QStandardPaths>

EXRWindow::EXRWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.layerList, SIGNAL(itemSelectionChanged()), this, SLOT(layerChanged()));
	connect(ui.resetViewButton, SIGNAL(clicked()), ui.imageWidget, SLOT(resetView()));
	connect(ui.originalScaleButton, SIGNAL(clicked()), ui.imageWidget, SLOT(zoomToOriginalSize()));
	connect(ui.exportImageButton, SIGNAL(clicked()), this, SLOT(exportImage()));

	mFile = std::make_unique<EXRFile>();
}

EXRWindow::~EXRWindow()
{
}

void EXRWindow::openFile(const QString& str)
{
	if (mFile->open(str)) {
		ui.sizeLabel->setText(QString("%1x%2").arg(mFile->width()).arg(mFile->height()));

		ui.layerList->clear();
		for (auto layer : mFile->layers()) {
			ui.layerList->addItem(layer->name());
		}

		if (!mFile->layers().empty())
			ui.layerList->setItemSelected(ui.layerList->item(0), true);

		setWindowTitle(QString("[EXR] %1").arg(str));
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

void EXRWindow::updateImage(int layerID)
{
	if (layerID >= mFile->layers().size())
		return;

	ui.imageWidget->setLayer(mFile->layers()[layerID]);
}

void EXRWindow::exportImage()
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