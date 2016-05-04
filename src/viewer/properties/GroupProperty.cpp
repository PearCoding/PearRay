#include "GroupProperty.h"

GroupProperty::GroupProperty(const QString& name) :
IProperty(name)
{
	makeHeader(true);
}

GroupProperty::~GroupProperty()
{
}

QString GroupProperty::valueText() const
{
	return "";
}

void GroupProperty::undo()
{
	setModified(false);
}

void GroupProperty::save()
{
	setModified(false);
}

QWidget* GroupProperty::editorWidget(QWidget* parent)
{
	return nullptr;
}
