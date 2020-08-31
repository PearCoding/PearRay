#include "IntProperty.h"

#include <QSpinBox>

namespace PRUI {
IntProperty::IntProperty()
	: IProperty()
	, mOldValue(0)
	, mValue(0)
	, mMaxValue(100000)
	, mMinValue(-100000)
	, mStepSize(1)
{
}

IntProperty::~IntProperty()
{
}

QString IntProperty::valueText() const
{
	return QString::number(mValue);
}

void IntProperty::undo()
{
	setValue(mOldValue);
	setModified(false);
}

void IntProperty::save()
{
	setDefaultValue(mValue);
	setModified(false);
}

QWidget* IntProperty::editorWidget(QWidget* parent)
{
	auto editor = new QSpinBox(parent);

	editor->setEnabled(isEnabled());
	editor->setReadOnly(isReadOnly());
	editor->setValue(mValue);
	editor->setMaximum(mMaxValue);
	editor->setMinimum(mMinValue);
	editor->setSingleStep(mStepSize);

	connect(editor, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));

	return editor;
}

void IntProperty::spinBoxChanged(int val)
{
	mValue = val;

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);

	emit valueChanged();
}

void IntProperty::setValue(int val)
{
	mValue = qMax(mMinValue, qMin(mMaxValue, val));

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);

	emit valueChanged();
}

int IntProperty::value() const
{
	return mValue;
}

void IntProperty::setDefaultValue(int val)
{
	mOldValue = qMax(mMinValue, qMin(mMaxValue, val));
	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);
}

int IntProperty::defaultValue() const
{
	return mOldValue;
}

int IntProperty::maxValue() const
{
	return mMaxValue;
}

int IntProperty::minValue() const
{
	return mMinValue;
}

int IntProperty::stepSize() const
{
	return mStepSize;
}

void IntProperty::setMaxValue(int i)
{
	mMaxValue = i;
}

void IntProperty::setMinValue(int i)
{
	mMinValue = i;
}

void IntProperty::setStepSize(int i)
{
	mStepSize = i;
}
} // namespace PRUI