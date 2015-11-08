#include "doubleproperty.h"

#include <QDoubleSpinBox>

DoubleProperty::DoubleProperty() :
IProperty(),
mSpinBox(nullptr),
mValue(0), mOldValue(0),
mMaxValue(100000), mMinValue(-100000), mStepSize(1), mDecimals(8)
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
	if (!mSpinBox)
	{
		mSpinBox = new QDoubleSpinBox(parent);

		connect(mSpinBox, SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged(double)));
	}

	mSpinBox->setEnabled(isEnabled());
	mSpinBox->setReadOnly(isReadOnly());
	mSpinBox->setValue(mValue);
	mSpinBox->setMaximum(mMaxValue);
	mSpinBox->setMinimum(mMinValue);
	mSpinBox->setSingleStep(mStepSize);
	mSpinBox->setDecimals(mDecimals);

	return mSpinBox;
}

void DoubleProperty::spinBoxChanged(double val)
{
	mValue = val;

	if (mValue != mOldValue && !isModified())
	{
		setModified(true);
	}
	else if (mValue == mOldValue && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void DoubleProperty::setValue(double val)
{
	mValue = qMax(mMinValue, qMin(mMaxValue, val));

	if (mValue != mOldValue && !isModified())
	{
		setModified(true);
	}
	else if (mValue == mOldValue && isModified())
	{
		setModified(false);
	}

	emit valueChanged();

	if (mSpinBox)
	{
		mSpinBox->setValue(mValue);
	}
}

double DoubleProperty::value() const
{
	return mValue;
}

void DoubleProperty::setDefaultValue(double val)
{
	mOldValue = qMax(mMinValue, qMin(mMaxValue, val));

	if (mValue != mOldValue && !isModified())
	{
		setModified(true);
	}
	else if (mValue == mOldValue && isModified())
	{
		setModified(false);
	}
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

	if (mSpinBox)
	{
		mSpinBox->setMaximum(i);
	}
}

void DoubleProperty::setMinValue(double i)
{
	mMinValue = i;

	if (mSpinBox)
	{
		mSpinBox->setMinimum(i);
	}
}

void DoubleProperty::setStepSize(double i)
{
	mStepSize = i;

	if (mSpinBox)
	{
		mSpinBox->setSingleStep(i);
	}
}

void DoubleProperty::setDecimals(int i)
{
	mDecimals = i;

	if (mSpinBox)
	{
		mSpinBox->setDecimals(i);
	}
}