#include "SelectionProperty.h"

#include <QComboBox>

SelectionProperty::SelectionProperty() :
IProperty(),
mWidget(nullptr),
mIndex(0), mDefaultIndex(0)
{
}

SelectionProperty::~SelectionProperty()
{
}

QString SelectionProperty::valueText() const
{
	return mData.isEmpty() ? "" : mData.at(mIndex).first;
}

void SelectionProperty::undo()
{
	setIndex(mDefaultIndex);
	setModified(false);
}

void SelectionProperty::save()
{
	setDefaultIndex(mIndex);
	setModified(false);
}

QWidget* SelectionProperty::editorWidget(QWidget* parent)
{
	if (!mWidget)
	{
		mWidget = new QComboBox(parent);

		typedef QPair<QString, QVariant> VData;
		foreach(VData pair, mData)
		{
			mWidget->addItem(pair.first, pair.second);
		}

		connect(mWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
	}

	mWidget->setEnabled(isEnabled());

	// TODO: changes in list?

	mWidget->setCurrentIndex(mIndex);

	return mWidget;
}

void SelectionProperty::comboChanged(int val)
{
	mIndex = val;

	if (mIndex != mDefaultIndex && !isModified())
	{
		setModified(true);
	}
	else if (mIndex == mDefaultIndex && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void SelectionProperty::setIndex(int val)
{
	mIndex = val;

	if (mIndex != mDefaultIndex && !isModified())
	{
		setModified(true);
	}
	else if (mIndex == mDefaultIndex && isModified())
	{
		setModified(false);
	}

	emit valueChanged();

	if (mWidget)
	{
		mWidget->setCurrentIndex(mIndex);
	}
}

int SelectionProperty::index() const
{
	return mIndex;
}

QVariant SelectionProperty::currentData() const
{
	return mData.at(mIndex).second;
}

QVariant SelectionProperty::data(int i) const
{
	return mData.at(i).second;
}

void SelectionProperty::setDefaultIndex(int val)
{
	mDefaultIndex = val;

	if (mIndex != mDefaultIndex && !isModified())
	{
		setModified(true);
	}
	else if (mIndex == mDefaultIndex && isModified())
	{
		setModified(false);
	}
}

int SelectionProperty::defaultIndex() const
{
	return mDefaultIndex;
}

void SelectionProperty::addItem(const QString& text, const QVariant& userData)
{
	mData.append(QPair<QString, QVariant>(text, userData));

	if (mWidget)
	{
		mWidget->addItem(text, userData);
	}
}

void SelectionProperty::removeItem(int index)
{
	mData.remove(index);

	if (mWidget)
	{
		mWidget->removeItem(index);
	}
}