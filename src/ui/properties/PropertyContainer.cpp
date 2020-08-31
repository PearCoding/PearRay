#include "PropertyContainer.h"

namespace PRUI {
PropertyContainer::PropertyContainer()
{
}

PropertyContainer::~PropertyContainer()
{
}

void PropertyContainer::add(IProperty* property)
{
	if (mAllProperties.contains(property))
		return;

	mTopProperties.append(property);

	rec_add(property);
}

void PropertyContainer::rec_add(IProperty* property)
{
	if (mAllProperties.contains(property))
		return;

	mAllProperties.append(property);

	connect(property, &IProperty::propertyDestroyed, [this](IProperty* property) { this->propertyWasDestroyed(property); });
	connect(property, &IProperty::propertyChanged, [this]() { this->propertyChanged((IProperty*)this->sender()); });
	connect(property, &IProperty::propertyStructureChanged, [this]() { this->propertyStructureChanged((IProperty*)this->sender()); });
	connect(property, &IProperty::valueChanged, [this]() { this->valueChanged((IProperty*)this->sender()); });

	for (IProperty* child : property->children())
		rec_add(child);
}

void PropertyContainer::remove(IProperty* property)
{
	if (!mAllProperties.contains(property))
		return;

	mTopProperties.removeOne(property);
	mAllProperties.removeOne(property);

	disconnect(property);

	for (IProperty* child : property->children())
		remove(child);
}

const QVector<IProperty*>& PropertyContainer::allProperties() const
{
	return mAllProperties;
}

const QVector<IProperty*>& PropertyContainer::topProperties() const
{
	return mTopProperties;
}

void PropertyContainer::propertyWasDestroyed(IProperty* prop)
{
	Q_ASSERT(prop);
	remove(prop);
}

void PropertyContainer::propertyWasChanged(IProperty* obj)
{
	emit propertyChanged(obj);
}

void PropertyContainer::propertyStructureWasChanged(IProperty* obj)
{
	emit propertyStructureChanged(obj);
}

void PropertyContainer::valueWasChanged(IProperty* obj)
{
	emit valueChanged(obj);
}
}