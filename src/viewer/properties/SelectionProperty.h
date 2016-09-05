#pragma once

#include "IProperty.h"

#include <QVector>
#include <QVariant>

class QComboBox;
class SelectionProperty : public IProperty
{
	Q_OBJECT
public:
	SelectionProperty(const QString& name, int index = 0);
	virtual ~SelectionProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	void setIndex(int i);
	int index() const;

	QVariant currentData() const;
	QVariant data(int i) const;

	void setDefaultIndex(int i);
	int defaultIndex() const;

	void addItem(const QString& text, const QVariant& userData = QVariant());
	void removeItem(int index);

private slots:
	void comboChanged(int i);

private: 
	QComboBox* mWidget;
	int mIndex;
	int mDefaultIndex;

	QVector<QPair<QString, QVariant> > mData;
};
