#pragma once

#include <QTreeView>

#include "PR_Config.h"

namespace PRUI {
class IProperty;
class PropertyContainer;
class PropertyItemDelegate;
class PropertyTreeModel;
class PR_LIB_UI PropertyView : public QTreeView {
	Q_OBJECT
public:
	explicit PropertyView(QWidget* parent = nullptr);
	virtual ~PropertyView();

	void setPropertyContainer(PropertyContainer* table);
	PropertyContainer* propertyContainer() const;

	void addProperty(IProperty* property);

protected:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;

private:
	PropertyTreeModel* mModel;
	PropertyItemDelegate* mDelegate;
};
} // namespace PRUI