#include "ProfWindow.h"
#include "io/ProfFile.h"
#include "prof/ProfTreeModel.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QMenu>

ProfWindow::ProfWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.splitter->setStretchFactor(0,10);
	//ui.splitter->setStretchFactor(1,1);

	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(onItemContextMenu(const QPoint&)));

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

	ui.treeView->setModel(mTreeModel.get());
	ui.treeView->expandToDepth(0);
	ui.treeView->resizeColumnToContents(0);
}

void ProfWindow::onItemContextMenu(const QPoint& point)
{
	QModelIndex index = ui.treeView->indexAt(point);
	if (!index.isValid())
		return;

	ProfTreeItem* item = reinterpret_cast<ProfTreeItem*>(index.internalPointer());

	QMenu contextMenu("Plot", this);
	QAction* action = new QAction("Show Plot", this);
	action->setCheckable(true);
	action->setChecked(ui.plotWidget->hasTimeGraph(item));
	action->setData(QVariant::fromValue(point));

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

	QPoint point = act->data().toPoint();

	QModelIndex index = ui.treeView->indexAt(point);
	if (!index.isValid())
		return;

	ProfTreeItem* item = reinterpret_cast<ProfTreeItem*>(index.internalPointer());

	if (!ui.plotWidget->hasTimeGraph(item))
		ui.plotWidget->addTimeGraph(item);
	else
		ui.plotWidget->removeTimeGraph(item);
}