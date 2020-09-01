#include "GroupProperty.h"

namespace PR {
namespace UI {
GroupProperty::GroupProperty()
	: IProperty()
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
	Q_UNUSED(parent);
	return nullptr;
}
} // namespace UI
} // namespace PR