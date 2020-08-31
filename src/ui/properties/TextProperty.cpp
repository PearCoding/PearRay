#include "TextProperty.h"

#include <QLineEdit>

namespace PRUI {
TextProperty::TextProperty()
	: IProperty()
	, mDefaultText()
	, mText()
{
}

TextProperty::~TextProperty()
{
}

QString TextProperty::valueText() const
{
	return mText;
}

void TextProperty::undo()
{
	setText(mDefaultText);
	setModified(false);
}

void TextProperty::save()
{
	setDefaultText(mText);
	setModified(false);
}

QWidget* TextProperty::editorWidget(QWidget* parent)
{
	auto editor = new QLineEdit(parent);

	editor->setEnabled(isEnabled());
	editor->setReadOnly(isReadOnly());
	editor->setText(mText);

	connect(editor, SIGNAL(textEdited(const QString&)), this, SLOT(lineChanged(const QString&)));

	return editor;
}

void TextProperty::lineChanged(const QString& val)
{
	mText = val;

	if (mText != mDefaultText && !isModified())
		setModified(true);
	else if (mText == mDefaultText && isModified())
		setModified(false);

	emit valueChanged();
}

void TextProperty::setText(const QString& val)
{
	mText = val;

	if (mText != mDefaultText && !isModified())
		setModified(true);
	else if (mText == mDefaultText && isModified())
		setModified(false);

	emit valueChanged();
}

QString TextProperty::text() const
{
	return mText;
}

void TextProperty::setDefaultText(const QString& val)
{
	mDefaultText = val;

	if (mText != mDefaultText && !isModified())
		setModified(true);
	else if (mText == mDefaultText && isModified())
		setModified(false);
}

QString TextProperty::defaultText() const
{
	return mDefaultText;
}
} // namespace PRUI