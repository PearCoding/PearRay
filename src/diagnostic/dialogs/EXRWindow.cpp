#include "EXRWindow.h"
#include "exr/EXRFile.h"

EXRWindow::EXRWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.layerList, SIGNAL(itemSelectionChanged()), this, SLOT(layerChanged()));

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