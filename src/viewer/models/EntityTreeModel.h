#pragma once

#include <QAbstractItemModel>

#include <memory>
namespace PR
{
	class VirtualEntity;
	class Scene;
}

class EntityTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit EntityTreeModel(const PR::Scene& scene, QObject* parent = nullptr);
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
	QList<std::shared_ptr<PR::VirtualEntity> > getEntities() const;

	const PR::Scene& mScene;
};