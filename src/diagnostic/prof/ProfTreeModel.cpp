#include "ProfTreeModel.h"
#include "ProfTreeItem.h"
#include "io/ProfFile.h"

ProfTreeModel::ProfTreeModel(const std::shared_ptr<ProfFile>& ctx)
	: mContext(ctx)
{
	setupData();
}

ProfTreeModel::~ProfTreeModel()
{
}

QVariant ProfTreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	ProfTreeItem* item = static_cast<ProfTreeItem*>(index.internalPointer());
	switch (role) {
	default:
		return QVariant();
	case Qt::DecorationRole:
		if (index.column() == 0 && !item->customIcon().isNull())
			return item->customIcon();
		else
			return QVariant();
	case Qt::DisplayRole:
		return item->data(index.column());
	}
}

Qt::ItemFlags ProfTreeModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractItemModel::flags(index);
}

QVariant ProfTreeModel::headerData(int section, Qt::Orientation orientation,
								   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case ProfTreeItem::C_Name:
			return "Name";
		case ProfTreeItem::C_TotalValue:
			return "Total Calls";
		case ProfTreeItem::C_TotalDuration:
			return "Total Duration";
		case ProfTreeItem::C_AverageDuration:
			return "Avg. Duration";
		default:
			return QVariant();
		}
	}

	return QVariant();
}

QModelIndex ProfTreeModel::index(int row, int column,
								 const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	ProfTreeItem* parentItem = parent.isValid() ? static_cast<ProfTreeItem*>(parent.internalPointer()) : nullptr;

	ProfTreeItem* childItem = nullptr;
	if (parentItem)
		childItem = parentItem->child(row).get();
	else if (row >= 0 && row < mFiles.size())
		childItem = mFiles.at(row).get();

	return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex ProfTreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	ProfTreeItem* childItem  = static_cast<ProfTreeItem*>(index.internalPointer());
	ProfTreeItem* parentItem = childItem->parent().get();

	return parentItem ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex();
}

int ProfTreeModel::rowCount(const QModelIndex& parent) const
{
	if (parent.column() > 0)
		return 0;

	ProfTreeItem* parentItem = parent.isValid() ? static_cast<ProfTreeItem*>(parent.internalPointer()) : nullptr;
	return parentItem ? parentItem->children().size() : mFiles.size();
}

int ProfTreeModel::columnCount(const QModelIndex&) const
{
	return ProfTreeItem::_C_COUNT;
}

void ProfTreeModel::setupData()
{
	// We expect each desc entry to be next to each same file entry!
	QString currentFile;
	std::shared_ptr<ProfTreeItem> fileParent;
	QHash<QString, std::shared_ptr<ProfTreeItem>> functionsPerFile;
	QHash<QString, std::shared_ptr<ProfTreeItem>> functionLinesPerFile;

	int counter = 0;
	for (const ProfEntry& entry : mContext->entries()) {
		if (entry.TimeCounterEntries.isEmpty()) {
			++counter;
			continue;
		}

		if (!fileParent || entry.File != currentFile) {
			fileParent  = std::make_shared<ProfTreeItem>(nullptr, entry.File, mContext.get());
			currentFile = entry.File;
			functionsPerFile.clear();
			functionLinesPerFile.clear();

			mFiles.push_back(fileParent);
		}

		std::shared_ptr<ProfTreeItem> functionItem;
		if (!functionsPerFile.contains(entry.Function)) {
			functionItem = std::make_shared<ProfTreeItem>(fileParent, entry.Function, mContext.get());
			fileParent->addChild(functionItem);
			functionsPerFile[entry.Function] = functionItem;
		} else {
			functionItem = functionsPerFile[entry.Function];
		}

		std::shared_ptr<ProfTreeItem> lineItem;
		QString funcLineName = QString("%1: %2").arg(entry.Function).arg(entry.Line);
		if (!functionLinesPerFile.contains(funcLineName)) {
			lineItem = std::make_shared<ProfTreeItem>(functionItem, QString("[%1]").arg(entry.Line), mContext.get());
			functionItem->addChild(lineItem);
			functionLinesPerFile[funcLineName] = lineItem;
		} else {
			lineItem = functionLinesPerFile[funcLineName];
		}

		std::shared_ptr<ProfTreeItem> entryItem = std::make_shared<ProfTreeItem>(lineItem,
																				 QString("(Thread) %1").arg(mContext->threadName(entry.ThreadID)),
																				 mContext.get(), counter);
		lineItem->addChild(entryItem);
		++counter;
	}

	// Simplify tree
	for (const auto& fileItem : mFiles) {
		for (const auto& funcItem : fileItem->children()) {
			if (funcItem->children().size() == 1) { // Only one line
				std::shared_ptr<ProfTreeItem> lineItem = funcItem->child(0);
				funcItem->removeChild(lineItem);
				funcItem->setName(QString("%1 %2").arg(funcItem->name()).arg(lineItem->name()));

				for (const auto& item : lineItem->children()) {
					item->setParent(funcItem);
					funcItem->addChild(item);
				}
			}
		}
	}
}