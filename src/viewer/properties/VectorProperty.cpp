#include "VectorProperty.h"

#include <QDoubleSpinBox>
#include <QBoxLayout>

VectorProperty::VectorProperty(const QString& name, int dimension) :
IProperty(name), mDimension(dimension)
{
	Q_ASSERT(dimension > 0 && dimension <= MAX_DIM);

	for (int i = 0; i < MAX_DIM; ++i)
	{
		mSpinBox[i] = nullptr;
		mValue[i] = 0; mOldValue[i] = 0; mMaxValue[i] = 100000; mMinValue[i] = -100000; mStepSize[i] = 1; mDecimals[i] = 2;
	}
}

VectorProperty::~VectorProperty()
{
	for (int i = 0; i < mDimension; ++i)
	{
		if (mSpinBox[i])
		{
			mSpinBox[i]->disconnect();
		}
	}
}

QString VectorProperty::valueText() const
{
	switch (mDimension)
	{
	case 1:
		return QString("%1").arg(mValue[0]);
	case 2:
		return QString("%1;%2").arg(mValue[0]).arg(mValue[1]);
	case 3:
		return QString("%1;%2;%3").arg(mValue[0]).arg(mValue[1]).arg(mValue[2]);
	case 4:
		return QString("%1;%2;%3;%4").arg(mValue[0]).arg(mValue[1]).arg(mValue[2]).arg(mValue[3]);
	default:
		return "INVALID";
	}
}

void VectorProperty::undo()
{
	for (int i = 0; i < mDimension; ++i)
	{
		setValue(mOldValue[i], i + 1);
	}
	setModified(false);
}

void VectorProperty::save()
{
	for (int i = 0; i < mDimension; ++i)
	{
		setDefaultValue(mValue[i], i + 1);
	}
	setModified(false);
}

QWidget* VectorProperty::editorWidget(QWidget* parent)
{
	if (!mSpinBox[0])
	{
		mSpinBox[0] = new QDoubleSpinBox(parent);

		connect(mSpinBox[0], SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged1(double)));
	}

	if (!mSpinBox[1] && mDimension >= 2)
	{
		mSpinBox[1] = new QDoubleSpinBox(parent);

		connect(mSpinBox[1], SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged2(double)));
	}

	if (!mSpinBox[2] && mDimension >= 3)
	{
		mSpinBox[2] = new QDoubleSpinBox(parent);

		connect(mSpinBox[2], SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged3(double)));
	}

	if (!mSpinBox[3] && mDimension == 4)
	{
		mSpinBox[3] = new QDoubleSpinBox(parent);

		connect(mSpinBox[3], SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged4(double)));
	}

	for (int i = 0; i < mDimension; ++i)
	{
		mSpinBox[i]->setEnabled(isEnabled());
		mSpinBox[i]->setReadOnly(isReadOnly());
		mSpinBox[i]->setValue(mValue[i]);
		mSpinBox[i]->setMaximum(mMaxValue[i]);
		mSpinBox[i]->setMinimum(mMinValue[i]);
		mSpinBox[i]->setSingleStep(mStepSize[i]);
		mSpinBox[i]->setDecimals(mDecimals[i]);
	}

	QWidget* widget = new QWidget(parent);
	QHBoxLayout* layout = new QHBoxLayout(parent);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	widget->setLayout(layout);

	for (int i = 0; i < mDimension; ++i)
	{
		layout->addWidget(mSpinBox[i]);
	}

	return widget;
}

void VectorProperty::spinBoxChanged1(double val)
{
	mValue[0] = val;

	if (mValue[0] != mOldValue[0] && !isModified())
	{
		setModified(true);
	}
	else if (mValue[0] == mOldValue[0] && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void VectorProperty::spinBoxChanged2(double val)
{
	mValue[1] = val;

	if (mValue[1] != mOldValue[1] && !isModified())
	{
		setModified(true);
	}
	else if (mValue[1] == mOldValue[1] && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void VectorProperty::spinBoxChanged3(double val)
{
	mValue[2] = val;

	if (mValue[2] != mOldValue[2] && !isModified())
	{
		setModified(true);
	}
	else if (mValue[2] == mOldValue[2] && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void VectorProperty::spinBoxChanged4(double val)
{
	mValue[3] = val;

	if (mValue[3] != mOldValue[3] && !isModified())
	{
		setModified(true);
	}
	else if (mValue[3] == mOldValue[3] && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void VectorProperty::setValue(double val, int dim)
{
	Q_ASSERT(dim > 0 && dim <= mDimension);

	mValue[dim-1] = qMax(mMinValue[dim - 1], qMin(mMaxValue[dim - 1], val));

	if (mValue[dim - 1] != mOldValue[dim - 1] && !isModified())
	{
		setModified(true);
	}
	else if (mValue[dim - 1] == mOldValue[dim - 1] && isModified())
	{
		setModified(false);
	}

	emit valueChanged();

	if (mSpinBox[dim - 1])
	{
		mSpinBox[dim - 1]->setValue(mValue[dim - 1]);
	}
}

double VectorProperty::value(int dim) const
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	return mValue[dim-1];
}

void VectorProperty::setDefaultValue(double val, int dim)
{
	Q_ASSERT(dim > 0 && dim <= mDimension);

	mOldValue[dim - 1] = qMax(mMinValue[dim - 1], qMin(mMaxValue[dim - 1], val));

	if (mValue[dim - 1] != mOldValue[dim - 1] && !isModified())
	{
		setModified(true);
	}
	else if (mValue[dim - 1] == mOldValue[dim - 1] && isModified())
	{
		setModified(false);
	}
}

double VectorProperty::defaultValue(int dim) const
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	return mOldValue[dim-1];
}

double VectorProperty::maxValue(int dim) const
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	return mMaxValue[dim-1];
}

double VectorProperty::minValue(int dim) const
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	return mMinValue[dim - 1];
}

double VectorProperty::stepSize(int dim) const
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	return mStepSize[dim - 1];
}

int VectorProperty::decimals(int dim) const
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	return mDecimals[dim - 1];
}

void VectorProperty::setMaxValue(double i, int dim)
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	mMaxValue[dim - 1] = i;

	if (mSpinBox[dim - 1])
	{
		mSpinBox[dim - 1]->setMaximum(i);
	}
}

void VectorProperty::setMinValue(double i, int dim)
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	mMinValue[dim - 1] = i;

	if (mSpinBox)
	{
		mSpinBox[dim - 1]->setMinimum(i);
	}
}

void VectorProperty::setStepSize(double i, int dim)
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	mStepSize[dim - 1] = i;

	if (mSpinBox)
	{
		mSpinBox[dim - 1]->setSingleStep(i);
	}
}

void VectorProperty::setDecimals(int i, int dim)
{
	Q_ASSERT(dim > 0 && dim <= mDimension);
	mDecimals[dim - 1] = i;

	if (mSpinBox)
	{
		mSpinBox[dim - 1]->setDecimals(i);
	}
}