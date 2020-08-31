#include "IProperty.h"

namespace PRUI {
IProperty::IProperty()
	: QObject()
	, mIsReadOnly(false)
	, mIsEnabled(true)
	, mIsModified(false)
	, mIsHeader(false)
	, mParent(nullptr)
{
}

IProperty::~IProperty()
{
	emit propertyDestroyed(this);
}

QString IProperty::toolTip() const
{
	return mToolTip;
}

QString IProperty::statusTip() const
{
	return mStatusTip;
}

QString IProperty::whatsThis() const
{
	return mWhatsThis;
}

QString IProperty::propertyName() const
{
	return mPropertyName;
}

bool IProperty::isReadOnly() const
{
	return mIsReadOnly;
}

bool IProperty::isEnabled() const
{
	return mIsEnabled;
}

bool IProperty::isModified() const
{
	return mIsModified;
}

bool IProperty::isHeader() const
{
	return mIsHeader;
}

void IProperty::setToolTip(const QString& str)
{
	mToolTip = str;
	emit propertyChanged();
}

void IProperty::setStatusTip(const QString& str)
{
	mStatusTip = str;
	emit propertyChanged();
}

void IProperty::setWhatsThis(const QString& str)
{
	mWhatsThis = str;
	emit propertyChanged();
}

void IProperty::setPropertyName(const QString& str)
{
	mPropertyName = str;
	emit propertyChanged();
}

void IProperty::setReadOnly(bool b)
{
	mIsReadOnly = b;
	emit propertyChanged();
}

void IProperty::setEnabled(bool b)
{
	mIsEnabled = b;
	emit propertyStructureChanged();
}

void IProperty::setModified(bool b)
{
	mIsModified = b;
	emit propertyChanged();
}

void IProperty::makeHeader(bool b)
{
	mIsHeader = b;
	emit propertyChanged();
}

void IProperty::addChild(IProperty* property)
{
	Q_ASSERT(property);
	mChilds.append(property);
	property->setParent(this);
	emit propertyStructureChanged();
}

void IProperty::removeChild(IProperty* property)
{
	Q_ASSERT(property);
	mChilds.removeAll(property);
	property->setParent(nullptr);
	emit propertyStructureChanged();
}

IProperty* IProperty::child(int i) const
{
	if (i < 0 || i >= mChilds.size())
		return nullptr;
	else
		return mChilds[i];
}

const QVector<IProperty*>& IProperty::children() const
{
	return mChilds;
}
} // namespace PRUI