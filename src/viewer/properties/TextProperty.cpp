#include "TextProperty.h"

#include <QLineEdit>

TextProperty::TextProperty() :
IProperty(),
mWidget(nullptr),
mText(), mDefaultText()
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
	if (!mWidget)
	{
		mWidget = new QLineEdit(parent);

		connect(mWidget, SIGNAL(textEdited(const QString&)), this, SLOT(lineChanged(const QString&)));
	}

	mWidget->setEnabled(isEnabled());
	mWidget->setReadOnly(isReadOnly());
	mWidget->setText(mText);

	return mWidget;
}

void TextProperty::lineChanged(const QString& val)
{
	mText = val;

	if (mText != mDefaultText && !isModified())
	{
		setModified(true);
	}
	else if (mText == mDefaultText && isModified())
	{
		setModified(false);
	}

	emit valueChanged();
}

void TextProperty::setText(const QString& val)
{
	mText = val;

	if (mText != mDefaultText && !isModified())
	{
		setModified(true);
	}
	else if (mText == mDefaultText && isModified())
	{
		setModified(false);
	}

	emit valueChanged();

	if (mWidget)
	{
		mWidget->setText(mText);
	}
}

QString TextProperty::text() const
{
	return mText;
}

void TextProperty::setDefaultText(const QString& val)
{
	mDefaultText = val;

	if (mText != mDefaultText && !isModified())
	{
		setModified(true);
	}
	else if (mText == mDefaultText && isModified())
	{
		setModified(false);
	}
}

QString TextProperty::defaultText() const
{
	return mDefaultText;
}