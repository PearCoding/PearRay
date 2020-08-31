#include "PropertyTreeModel.h"
#include "IProperty.h"
#include "PropertyContainer.h"

namespace PRUI {
PropertyTreeModel::PropertyTreeModel(PropertyContainer* cnt)
	: mContainer(cnt)
{
}

PropertyTreeModel::~PropertyTreeModel()
{
}

QVariant PropertyTreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	IProperty* item = static_cast<IProperty*>(index.internalPointer());
	switch (role) {
	default:
		return QVariant();
	case Qt::ToolTipRole:
		if (index.column() == 0)
			return item->toolTip();
		else
			return QVariant();
	case Qt::WhatsThisRole:
		if (index.column() == 0)
			return item->whatsThis();
		else
			return QVariant();
	case Qt::StatusTipRole:
		if (index.column() == 0)
			return item->statusTip();
		else
			return QVariant();
	case Qt::DisplayRole:
		if (index.column() == 0)
			return item->propertyName();
		else
			return item->valueText();
	case Qt::BackgroundRole:
	case Qt::UserRole: // Return void pointer to item
		return QVariant::fromValue<void*>(static_cast<void*>(item));
	}
}

bool PropertyTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::EditRole)
		return false;

	if (!index.isValid())
		return false;

	IProperty* childItem = static_cast<IProperty*>(index.internalPointer());
	if (childItem->isReadOnly())
		return false;

	if (index.column() == 0) { // Name
		childItem->setPropertyName(value.toString());
		emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
		return true;
	} else if (index.column() == 1) { // Value
									  // TODO
	}
	return false;
}

Qt::ItemFlags PropertyTreeModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	IProperty* childItem = static_cast<IProperty*>(index.internalPointer());
	Qt::ItemFlags flags	 = Qt::ItemIsSelectable;

	if (childItem->children().empty())
		flags |= Qt::ItemNeverHasChildren;
	if (!childItem->isReadOnly())
		flags |= Qt::ItemIsEditable;
	if (childItem->isEnabled())
		flags |= Qt::ItemIsEnabled;

	return flags;
}

QVariant PropertyTreeModel::headerData(int section, Qt::Orientation orientation,
									   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("Name");
		case 1:
			return tr("Value");
		default:
			return QVariant();
		}
	}

	return QVariant();
}

QModelIndex PropertyTreeModel::index(int row, int column,
									 const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	IProperty* parentItem = parent.isValid() ? static_cast<IProperty*>(parent.internalPointer()) : nullptr;

	IProperty* childItem = nullptr;
	if (parentItem)
		childItem = parentItem->child(row);
	else if (row >= 0 && row < mContainer->topProperties().size())
		childItem = mContainer->topProperties()[row];

	return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex PropertyTreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	IProperty* childItem  = static_cast<IProperty*>(index.internalPointer());
	IProperty* parentItem = childItem->parent();

	if (parentItem) {
		int row = 0;
		if (parentItem->parent())
			row = parentItem->parent()->children().indexOf(parentItem);
		return createIndex(row, 0, parentItem);
	}
	return QModelIndex();
}

int PropertyTreeModel::rowCount(const QModelIndex& parent) const
{
	if (parent.column() > 0)
		return 0;

	IProperty* parentItem = parent.isValid() ? static_cast<IProperty*>(parent.internalPointer()) : nullptr;
	return parentItem ? parentItem->children().size() : mContainer->topProperties().size();
}

int PropertyTreeModel::columnCount(const QModelIndex&) const
{
	return 2;
}

void PropertyTreeModel::addProperty(IProperty* property)
{
	int row = mContainer->topProperties().size();
	beginInsertRows(QModelIndex(), row, row);
	mContainer->add(property);
	endInsertRows();
}
} // namespace PRUI