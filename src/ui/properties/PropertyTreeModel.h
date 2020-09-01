#pragma once

#include "PR_Config.h"
#include <QAbstractItemModel>

namespace PR {
namespace UI {
class IProperty;
class PropertyContainer;
class PR_LIB_UI PropertyTreeModel : public QAbstractItemModel {
public:
	PropertyTreeModel(PropertyContainer* cnt);
	virtual ~PropertyTreeModel();

	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value,
				 int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column,
					  const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	void addProperty(IProperty* property);
	inline PropertyContainer* container() const { return mContainer; }

private:
	PropertyContainer* mContainer;
};
} // namespace UI
} // namespace PR