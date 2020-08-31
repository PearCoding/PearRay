#include "ProfWindow.h"
#include "io/ProfFile.h"
#include "prof/ProfTreeModel.h"

#include <QFileDialog>
#include <QMenu>
#include <QPixmap>
#include <QSortFilterProxyModel>
#include <QStandardPaths>

ProfWindow::ProfWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.splitter->setStretchFactor(0, 10);

	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.treeView->setSortingEnabled(true);
	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(onItemContextMenu(const QPoint&)));

	ui.plotWidget->setShowMode(ui.showModeCB->currentIndex()); // Enforce to be up to date with the ui
	connect(ui.showModeCB, SIGNAL(currentIndexChanged(int)),
			ui.plotWidget, SLOT(setShowMode(int)));
	connect(ui.resetViewButton, SIGNAL(clicked()), ui.plotWidget, SLOT(resetView()));
	connect(ui.exportImageButton, SIGNAL(clicked()), this, SLOT(exportImage()));
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
		setWindowTitle(QString("[Profiler] %1").arg(str));
	}
}

void ProfWindow::setupModel()
{
	mTreeModel = std::make_shared<ProfTreeModel>(mContext);

	QSortFilterProxyModel* sortProxy = new QSortFilterProxyModel(this);
	sortProxy->setSourceModel(mTreeModel.get());

	ui.treeView->setModel(sortProxy);
	ui.treeView->expandToDepth(0);
	for (int i = 0; i < mTreeModel->columnCount(); ++i)
		ui.treeView->resizeColumnToContents(i);
}

void ProfWindow::exportImage()
{
	const QStringList loc = QStandardPaths::standardLocations(
		QStandardPaths::PicturesLocation);

	QString file = QFileDialog::getSaveFileName(this, tr("Save Plot as Image"),
												loc.isEmpty()
													? QDir::currentPath()
													: loc.last(),
												tr("Images (*.png *.xpm *.jpg)"));

	if (!file.isEmpty()) {
		QPixmap image = ui.plotWidget->grab();
		image.save(file);
	}
}

void ProfWindow::onItemContextMenu(const QPoint& point)
{
	if (mContext->entryCount() == 0)
		return;

	QMenu contextMenu("Plot", this);

	// Item dependent actions
	QModelIndex index = ui.treeView->indexAt(point);
	if (index.isValid()) {
		ProfTreeItem* item = static_cast<ProfTreeItem*>(index.data(Qt::UserRole).value<void*>());
		if (item) {
			QAction* actionShow = new QAction("Show Plot", this);
			actionShow->setCheckable(true);
			actionShow->setChecked(ui.plotWidget->hasTimeGraph(item));

			connect(actionShow, &QAction::triggered, [=]() {
				if (!ui.plotWidget->hasTimeGraph(item))
					ui.plotWidget->addTimeGraph(item);
				else
					ui.plotWidget->removeTimeGraph(item);
			});

			contextMenu.addAction(actionShow);
			contextMenu.addSeparator();
		}
	}

	// General actions
	QAction* actionCollapseAll = new QAction("Collapse All", this);
	connect(actionCollapseAll, &QAction::triggered, ui.treeView, &QTreeView::collapseAll);
	contextMenu.addAction(actionCollapseAll);

	QAction* actionExpandFunctions = new QAction("Expand Functions", this);
	connect(actionExpandFunctions, &QAction::triggered, [=]() { ui.treeView->expandToDepth(0); });
	contextMenu.addAction(actionExpandFunctions);

	QAction* actionExpandAll = new QAction("Expand All", this);
	connect(actionExpandAll, &QAction::triggered, ui.treeView, &QTreeView::expandAll);
	contextMenu.addAction(actionExpandAll);

	contextMenu.exec(ui.treeView->mapToGlobal(point));
}