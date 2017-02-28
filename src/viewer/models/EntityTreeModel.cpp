#include "EntityTreeModel.h"


#include "scene/Scene.h"
#include "entity/Entity.h"
#include "entity/RenderEntity.h"

EntityTreeModel::EntityTreeModel(const PR::Scene& scene, QObject* parent)
	: QAbstractItemModel(parent), mScene(scene)
{
}

EntityTreeModel::~EntityTreeModel()
{
}

QVariant EntityTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	PR::Entity *item = static_cast<PR::Entity*>(index.internalPointer());
	if (role == Qt::ToolTipRole)
	{
		QString tooltip = QString("World coordinates:\nPos\t[%1, %2, %3]\nRot\t[%4, %5, %6, %7]\nScale\t[%8, %9, %10]")
			.arg(PM::pm_GetX(item->position())).arg(PM::pm_GetY(item->position())).arg(PM::pm_GetZ(item->position()))
			.arg(PM::pm_GetX(item->rotation())).arg(PM::pm_GetY(item->rotation())).arg(PM::pm_GetZ(item->rotation())).arg(PM::pm_GetW(item->rotation()))
			.arg(PM::pm_GetX(item->scale())).arg(PM::pm_GetY(item->scale())).arg(PM::pm_GetZ(item->scale()));

		PR::RenderEntity* entity = dynamic_cast<PR::RenderEntity*>(item);
		if (entity)
		{
			auto box = entity->worldBoundingBox();
			tooltip += QString("\nBoundingBox\t[%1, %2, %3]").arg(box.width()).arg(box.height()).arg(box.depth());
		}
		return tooltip;
	}
	else if(role == Qt::DisplayRole)
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
	else
	{
		return QVariant();
	}
}

Qt::ItemFlags EntityTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

QVariant EntityTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return section == 0 ? tr("Name") : tr("Type");
	else
		return QVariant();
}

QModelIndex EntityTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	auto list = getEntities();

	if (list.size() > row)
	{
		auto childItem = list.at(row);
		return createIndex(row, column, childItem.get());
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex EntityTreeModel::parent(const QModelIndex &index) const
{
	return QModelIndex();
}

int EntityTreeModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	return getEntities().size();
}

int EntityTreeModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

QList<std::shared_ptr<PR::Entity> > EntityTreeModel::getEntities() const
{
	return QList<std::shared_ptr<PR::Entity> >::fromStdList(mScene.entities());
}