#include "SelectionProperty.h"

#include <QComboBox>

namespace PR {
namespace UI {
SelectionProperty::SelectionProperty()
	: IProperty()
	, mDefaultIndex(0)
	, mIndex(0)
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
	auto editor = new QComboBox(parent);

	editor->setEnabled(isEnabled());

	typedef QPair<QString, QVariant> VData;
	for (VData pair : mData)
		editor->addItem(pair.first, pair.second);

	editor->setCurrentIndex(mIndex);

	connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
	return editor;
}

void SelectionProperty::comboChanged(int val)
{
	mIndex = val;

	if (mIndex != mDefaultIndex && !isModified())
		setModified(true);
	else if (mIndex == mDefaultIndex && isModified())
		setModified(false);

	emit valueChanged();
}

void SelectionProperty::setIndex(int val)
{
	mIndex = val;

	if (mIndex != mDefaultIndex && !isModified())
		setModified(true);
	else if (mIndex == mDefaultIndex && isModified())
		setModified(false);

	emit valueChanged();
}

int SelectionProperty::index() const
{
	return mIndex;
}

void SelectionProperty::setDefaultIndex(int val)
{
	mDefaultIndex = val;

	if (mIndex != mDefaultIndex && !isModified())
		setModified(true);
	else if (mIndex == mDefaultIndex && isModified())
		setModified(false);
}

int SelectionProperty::defaultIndex() const
{
	return mDefaultIndex;
}

void SelectionProperty::addItem(const QString& text, const QVariant& userData)
{
	mData.append(QPair<QString, QVariant>(text, userData));
}

void SelectionProperty::removeItem(int index)
{
	mData.removeAt(index);
}
} // namespace UI
} // namespace PR