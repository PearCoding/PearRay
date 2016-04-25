#include "EntityTreeModel.h"


#include "scene/Scene.h"
#include "entity/Entity.h"

EntityTreeModel::EntityTreeModel(PR::Scene* scene, QObject  *parent)
	: QAbstractItemModel(parent), mScene(scene)
{
}

EntityTreeModel::~EntityTreeModel()
{
}

QVariant EntityTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
	{
		return QVariant();
	}

	PR::Entity *item = static_cast<PR::Entity*>(index.internalPointer());
	if (role == Qt::ToolTipRole)
	{
		return QString("World coordinates:\nPos\t[%1, %2, %3]\nRot\t[%4, %5, %6, %7]\nScale\t[%8, %9, %10]")
			.arg(PM::pm_GetX(item->position())).arg(PM::pm_GetY(item->position())).arg(PM::pm_GetZ(item->position()))
			.arg(PM::pm_GetX(item->rotation())).arg(PM::pm_GetY(item->rotation())).arg(PM::pm_GetZ(item->rotation())).arg(PM::pm_GetZ(item->rotation()))
			.arg(PM::pm_GetX(item->scale())).arg(PM::pm_GetY(item->scale())).arg(PM::pm_GetZ(item->scale()));
	}
	else
	{
		switch (index.column())
		{
		case 0:
			return item->name().c_str();
		case 1:
			return QString(item->type().c_str()).toUpper();
		default:
			return "";
		}
	}
}

Qt::ItemFlags EntityTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
	{
		return 0;
	}

	return QAbstractItemModel::flags(index);
}

QVariant EntityTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		return section == 0 ? tr("Name") : tr("Type");
	}
	return QVariant();
}

QModelIndex EntityTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	PR::Entity* parentItem;

	if (!parent.isValid())
	{
		parentItem = nullptr;
	}
	else
	{
		parentItem = static_cast<PR::Entity*>(parent.internalPointer());
	}

	auto list = getChildren(parentItem);

	if (list.size() > row)
	{
		PR::Entity* childItem = list.at(row);
		return createIndex(row, column, childItem);
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex EntityTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
	{
		return QModelIndex();
	}

	PR::Entity *childItem = static_cast<PR::Entity*>(index.internalPointer());
	PR::Entity *parentItem = childItem->parent();

	if (parentItem == nullptr)
	{
		return QModelIndex();
	}

	int row = 0;

	if (parentItem->parent())
	{
		auto list = getChildren(parentItem->parent());
		row = list.indexOf(parentItem);
	}

	return createIndex(row, 0, parentItem);
}

int EntityTreeModel::rowCount(const QModelIndex &parent) const
{
	PR::Entity *parentItem;
	if (parent.column() > 0)
	{
		return 0;
	}

	if (!parent.isValid())
	{
		return getChildren(nullptr).size();
	}
	else
	{
		return getChildren(static_cast<PR::Entity*>(parent.internalPointer())).size();
	}
}

int EntityTreeModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

QList<PR::Entity*> EntityTreeModel::getChildren(PR::Entity* entity) const
{
	QList<PR::Entity*> children;

	for (PR::Entity* e : mScene->entities())
	{
		if (e->parent() == entity)
		{
			children.push_back(e);
		}
	}

	return children;
}