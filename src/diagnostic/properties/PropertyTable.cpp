#include "PropertyTable.h"

PropertyTable::PropertyTable()
{
	connect(&mPropertyChangedMapper, SIGNAL(mapped(QObject*)), this, SLOT(propertyWasChanged(QObject*)));
	connect(&mPropertyStructureChangedMapper, SIGNAL(mapped(QObject*)), this, SLOT(propertyStructureWasChanged(QObject*)));
	connect(&mValueChangedMapper, SIGNAL(mapped(QObject*)), this, SLOT(valueWasChanged(QObject*)));
}

PropertyTable::~PropertyTable()
{
}

void PropertyTable::add(IProperty* property)
{
	if (mAllProperties.contains(property)) {
		return;
	}

	mTopProperties.append(property);

	rec_add(property);
}

void PropertyTable::rec_add(IProperty* property)
{
	if (mAllProperties.contains(property)) {
		return;
	}

	mAllProperties.append(property);

	connect(property, SIGNAL(propertyDestroyed(IProperty*)), this, SLOT(propertyWasDestroyed(IProperty*)));
	connect(property, SIGNAL(propertyChanged()), &mPropertyChangedMapper, SLOT(map()));
	connect(property, SIGNAL(propertyStructureChanged()), &mPropertyStructureChangedMapper, SLOT(map()));
	connect(property, SIGNAL(valueChanged()), &mValueChangedMapper, SLOT(map()));

	foreach (IProperty* child, property->childs()) {
		rec_add(child);
	}
}

void PropertyTable::remove(IProperty* property)
{
	if (!mAllProperties.contains(property)) {
		return;
	}

	mTopProperties.removeOne(property);
	mAllProperties.removeOne(property);

	disconnect(property, SIGNAL(propertyDestroyed(IProperty*)), this, SLOT(propertyWasDestroyed(IProperty*)));
	disconnect(property, SIGNAL(propertyChanged()), &mPropertyChangedMapper, SLOT(map()));
	disconnect(property, SIGNAL(propertyStructureChanged()), &mPropertyStructureChangedMapper, SLOT(map()));
	disconnect(property, SIGNAL(valueChanged()), &mValueChangedMapper, SLOT(map()));

	foreach (IProperty* child, property->childs()) {
		remove(child);
	}
}

QList<IProperty*> PropertyTable::allProperties() const
{
	return mAllProperties;
}

QList<IProperty*> PropertyTable::topProperties() const
{
	return mTopProperties;
}

void PropertyTable::propertyWasDestroyed(IProperty* prop)
{
	Q_ASSERT(prop);
	remove(prop);
}

void PropertyTable::propertyWasChanged(QObject* obj)
{
	emit propertyChanged(static_cast<IProperty*>(obj));
}

void PropertyTable::propertyStructureWasChanged(QObject* obj)
{
	emit propertyStructureChanged(static_cast<IProperty*>(obj));
}

void PropertyTable::valueWasChanged(QObject* obj)
{
	emit valueChanged(static_cast<IProperty*>(obj));
}
