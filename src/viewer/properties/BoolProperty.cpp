#include "BoolProperty.h"

#include <QCheckBox>

BoolProperty::BoolProperty(const QString& name, bool value) :
IProperty(name),
mCheckBox(nullptr),
mValue(value), mOldValue(value)
{
}

BoolProperty::~BoolProperty()
{
}

QString BoolProperty::valueText() const
{
	return QString::number(mValue);
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
	if (!mCheckBox)
	{
		mCheckBox = new QCheckBox(parent);

		connect(mCheckBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
	}

	mCheckBox->setEnabled(isEnabled() || isReadOnly());
	mCheckBox->setChecked(mValue);

	return mCheckBox;
}

void BoolProperty::checkBoxChanged(int val)
{
	mValue = (val == Qt::Checked);

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

void BoolProperty::setValue(bool val)
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

	if (mCheckBox)
	{
		mCheckBox->setChecked(val);
	}
}

bool BoolProperty::value() const
{
	return mValue;
}

void BoolProperty::setDefaultValue(bool val)
{
	mOldValue = val;

	if (mValue != mOldValue && !isModified())
	{
		setModified(true);
	}
	else if (mValue == mOldValue && isModified())
	{
		setModified(false);
	}
}

bool BoolProperty::defaultValue() const
{
	return mOldValue;
}