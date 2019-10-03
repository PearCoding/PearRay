#pragma once

#include <QList>
#include <QSignalMapper>
#include <QString>

#include "IProperty.h"

class PropertyTable : public QObject {
	Q_OBJECT
public:
	PropertyTable();
	~PropertyTable();

	void add(IProperty* property);
	void remove(IProperty* property);

	QList<IProperty*> topProperties() const;
	QList<IProperty*> allProperties() const;

signals:
	void propertyChanged(IProperty* prop);
	void propertyStructureChanged(IProperty* prop);
	void valueChanged(IProperty* prop);

private slots:
	void propertyWasDestroyed(IProperty* prop);
	void propertyWasChanged(QObject* obj);
	void propertyStructureWasChanged(QObject* obj);
	void valueWasChanged(QObject* obj);

private:
	void rec_add(IProperty* property);

	QList<IProperty*> mAllProperties;
	QList<IProperty*> mTopProperties;
	QSignalMapper mPropertyChangedMapper;
	QSignalMapper mPropertyStructureChangedMapper;
	QSignalMapper mValueChangedMapper;
};
