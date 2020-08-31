#pragma once

#include <QTreeWidget>

#include "PR_Config.h"

namespace PRUI {
class IProperty;
class PropertyTable;
class PR_LIB_UI PropertyView : public QTreeWidget {
	Q_OBJECT
public:
	explicit PropertyView(QWidget* parent = nullptr);
	virtual ~PropertyView();

	void setPropertyTable(PropertyTable* table);
	PropertyTable* propertyTable() const;

public slots:
	void reset();

protected:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;

private:
	void addChildItems(QTreeWidgetItem* parent, IProperty*);
	QTreeWidgetItem* setupItem(QTreeWidgetItem* item, IProperty* property);

	PropertyTable* mProperties;
	QMap<QTreeWidgetItem*, IProperty*> mMapper;
};
}