#include "PropertyTable.h"

namespace PRUI {
PropertyTable::PropertyTable()
{
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
	if (mAllProperties.contains(property))
		return;

	mAllProperties.append(property);

	connect(property, &IProperty::propertyDestroyed, [this](IProperty* property) { this->propertyWasDestroyed(property); });
	connect(property, &IProperty::propertyChanged, [this]() { this->propertyChanged((IProperty*)this->sender()); });
	connect(property, &IProperty::propertyStructureChanged, [this]() { this->propertyStructureChanged((IProperty*)this->sender()); });
	connect(property, &IProperty::valueChanged, [this]() { this->valueChanged((IProperty*)this->sender()); });

	foreach (IProperty* child, property->childs())
		rec_add(child);
}

void PropertyTable::remove(IProperty* property)
{
	if (!mAllProperties.contains(property))
		return;

	mTopProperties.removeOne(property);
	mAllProperties.removeOne(property);

	disconnect(property);

	foreach (IProperty* child, property->childs())
		remove(child);
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

void PropertyTable::propertyWasChanged(IProperty* obj)
{
	emit propertyChanged(obj);
}

void PropertyTable::propertyStructureWasChanged(IProperty* obj)
{
	emit propertyStructureChanged(obj);
}

void PropertyTable::valueWasChanged(IProperty* obj)
{
	emit valueChanged(obj);
}
}