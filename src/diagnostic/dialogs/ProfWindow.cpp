#include "ProfWindow.h"
#include "io/ProfFile.h"
#include "prof/ProfTreeModel.h"

#include <QFileDialog>
#include <QStandardPaths>

ProfWindow::ProfWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ProfWindow::~ProfWindow()
{
}

void ProfWindow::openFile(const QString& str)
{
	std::shared_ptr<ProfFile> file = std::make_shared<ProfFile>();
	if (file->open(str)) {
		mContext = file;
		setupModel();
		setupThreadList();
		setupPlot();
		setWindowTitle(QString("[Profiler] %1").arg(str));
	}
}

void ProfWindow::setupModel()
{
	mTreeModel = std::make_shared<ProfTreeModel>(mContext);

	ui.treeView->setModel(mTreeModel.get());
	ui.treeView->expandToDepth(0);
	ui.treeView->resizeColumnToContents(0);
	ui.treeView->setSortingEnabled(true);
}

void ProfWindow::setupThreadList()
{
	ui.accumThreadsCB->setChecked(true);
	ui.threadListWidget->clear();

	for (const QString& name : mContext->threadNames()) {
		QListWidgetItem* item = new QListWidgetItem(name);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		ui.threadListWidget->addItem(item);
	}
}

void ProfWindow::setupPlot()
{
}