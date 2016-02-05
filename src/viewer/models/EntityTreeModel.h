#pragma once

#include <QAbstractItemModel>

namespace PR
{
	class Entity;
	class Scene;
}

class EntityTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit EntityTreeModel(PR::Scene* scene, QObject  *parent = 0);
	virtual ~EntityTreeModel();
	
	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
	QList<PR::Entity*> getChildren(PR::Entity* entity) const;

	PR::Scene* mScene;
};