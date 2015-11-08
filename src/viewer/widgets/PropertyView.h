#pragma once

#include <QTreeWidget>

class IProperty;
class PropertyTable;
class PropertyView : public QTreeWidget
{
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
	void drawBranches(QPainter* painter, const QRect & rect, const QModelIndex & index) const;

private slots:
	void propertyStructureWasChanged(IProperty* obj);

private:
	void addChildItems(QTreeWidgetItem* parent, IProperty*);
	QTreeWidgetItem* setupItem(QTreeWidgetItem* item, IProperty* property);

	PropertyTable* mProperties;
	QMap<QTreeWidgetItem*, IProperty*> mMapper;
};
