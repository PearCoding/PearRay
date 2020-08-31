#include "DoubleProperty.h"

#include <QDoubleSpinBox>

namespace PRUI {
DoubleProperty::DoubleProperty()
	: IProperty()
	, mOldValue(0)
	, mValue(0)
	, mMaxValue(100000)
	, mMinValue(-100000)
	, mStepSize(1)
	, mDecimals(8)
{
}

DoubleProperty::~DoubleProperty()
{
}

QString DoubleProperty::valueText() const
{
	return QString::number(mValue);
}

void DoubleProperty::undo()
{
	setValue(mOldValue);
	setModified(false);
}

void DoubleProperty::save()
{
	setDefaultValue(mValue);
	setModified(false);
}

QWidget* DoubleProperty::editorWidget(QWidget* parent)
{
	auto editor = new QDoubleSpinBox(parent);

	editor->setEnabled(isEnabled());
	editor->setReadOnly(isReadOnly());
	editor->setValue(mValue);
	editor->setMaximum(mMaxValue);
	editor->setMinimum(mMinValue);
	editor->setSingleStep(mStepSize);
	editor->setDecimals(mDecimals);

	connect(editor, SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged(double)));

	return editor;
}

void DoubleProperty::spinBoxChanged(double val)
{
	mValue = val;

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);

	emit valueChanged();
}

void DoubleProperty::setValue(double val)
{
	mValue = qMax(mMinValue, qMin(mMaxValue, val));

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);

	emit valueChanged();
}

double DoubleProperty::value() const
{
	return mValue;
}

void DoubleProperty::setDefaultValue(double val)
{
	mOldValue = qMax(mMinValue, qMin(mMaxValue, val));

	if (mValue != mOldValue && !isModified())
		setModified(true);
	else if (mValue == mOldValue && isModified())
		setModified(false);
}

double DoubleProperty::defaultValue() const
{
	return mOldValue;
}

double DoubleProperty::maxValue() const
{
	return mMaxValue;
}

double DoubleProperty::minValue() const
{
	return mMinValue;
}

double DoubleProperty::stepSize() const
{
	return mStepSize;
}

int DoubleProperty::decimals() const
{
	return mDecimals;
}

void DoubleProperty::setMaxValue(double i)
{
	mMaxValue = i;
}

void DoubleProperty::setMinValue(double i)
{
	mMinValue = i;
}

void DoubleProperty::setStepSize(double i)
{
	mStepSize = i;
}

void DoubleProperty::setDecimals(int i)
{
	mDecimals = i;
}
} // namespace PRUI