#include "BoolProperty.h"

#include <QCheckBox>

namespace PRUI {
BoolProperty::BoolProperty()
	: IProperty()
	, mOldValue(false)
	, mValue(false)
{
}

BoolProperty::~BoolProperty()
{
}

QString BoolProperty::valueText() const
{
	return mValue ? tr("true") : tr("false");
}

void BoolProperty::undo()
{
	setValue(mOldValue);
	setModified(false);
}

void BoolProperty::save()
{
	setDefaultValue(mValue);
	setModified(false);
}

QWidget* BoolProperty::editorWidget(QWidget* parent)
{
	auto widget = new QCheckBox(parent);

	widget->setEnabled(isEnabled() && !isReadOnly());
	widget->setCheckState(mValue ? Qt::Checked : Qt::Unchecked);

	connect(widget, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
	return widget;
}

void BoolProperty::stateChanged(int state)
{
	mValue = (state != Qt::Unchecked);

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);

	emit valueChanged();
}

void BoolProperty::setValue(bool val)
{
	mValue = val;

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);

	emit valueChanged();
}

bool BoolProperty::value() const
{
	return mValue;
}

void BoolProperty::setDefaultValue(bool val)
{
	mOldValue = val;

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);
}

bool BoolProperty::defaultValue() const
{
	return mOldValue;
}
} // namespace PRUI