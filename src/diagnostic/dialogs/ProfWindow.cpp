#include "ProfWindow.h"
#include "io/ProfFile.h"
#include "prof/ProfTreeModel.h"

#include <QFileDialog>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QPixmap>

ProfWindow::ProfWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.splitter->setStretchFactor(0, 10);
	//ui.splitter->setStretchFactor(1,1);

	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.treeView->setSortingEnabled(true);
	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(onItemContextMenu(const QPoint&)));

	connect(ui.showModeCB, SIGNAL(currentIndexChanged(int)),
			ui.plotWidget, SLOT(setShowMode(int)));
	connect(ui.resetViewButton, SIGNAL(clicked()), ui.plotWidget, SLOT(resetView()));
	connect(ui.exportImageButton, SIGNAL(clicked()), this, SLOT(exportImage()));

	connect(&mSignalMapper, SIGNAL(mapped(QObject*)),
			this, SLOT(onItemShowPlot(QObject*)));
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
	ui.treeView->resizeColumnToContents(0);
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
	QModelIndex index = ui.treeView->indexAt(point);
	if (!index.isValid())
		return;

	ProfTreeItem* item = static_cast<ProfTreeItem*>(index.data(Qt::UserRole).value<void*>());
	if (!item)
		return;

	QMenu contextMenu("Plot", this);
	QAction* action = new QAction("Show Plot", this);
	action->setCheckable(true);
	action->setChecked(ui.plotWidget->hasTimeGraph(item));
	action->setData(QVariant::fromValue(static_cast<void*>(item)));

	connect(action, SIGNAL(triggered()), &mSignalMapper, SLOT(map()));
	mSignalMapper.setMapping(action, action);

	contextMenu.addAction(action);
	contextMenu.exec(ui.treeView->mapToGlobal(point));
}

void ProfWindow::onItemShowPlot(QObject* obj)
{
	QAction* act = qobject_cast<QAction*>(obj);
	if (!act)
		return;

	ProfTreeItem* item = static_cast<ProfTreeItem*>(act->data().value<void*>());
	if (!item)
		return;

	if (!ui.plotWidget->hasTimeGraph(item))
		ui.plotWidget->addTimeGraph(item);
	else
		ui.plotWidget->removeTimeGraph(item);
}