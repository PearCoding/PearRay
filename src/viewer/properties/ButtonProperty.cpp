#include "ButtonProperty.h"

ButtonProperty::ButtonProperty(const QString& name) :
	IProperty(name),
	mWidget(nullptr)
{
	makeNoName(true);
}

ButtonProperty::~ButtonProperty()
{
}

QString ButtonProperty::valueText() const
{
	return propertyName();
}

void ButtonProperty::undo()
{
	setModified(false);
}

void ButtonProperty::save()
{
	setModified(false);
}

QWidget* ButtonProperty::editorWidget(QWidget* parent)
{
	if (!mWidget)
	{
		mWidget = new QPushButton(parent);

		connect(mWidget, SIGNAL(clicked()), this, SIGNAL(valueChanged()));
	}

	mWidget->setText(valueText());
	mWidget->setEnabled(isEnabled());
	//mWidget->setFlat(true);

	return mWidget;
}