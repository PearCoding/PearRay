#include "ColorProperty.h"
#include "IntProperty.h"

#include "widgets/ColorButton.h"

namespace PRUI {
ColorProperty::ColorProperty()
	: IProperty()
	, mWidget(nullptr)
	, mDefaultColor(Qt::white)
	, mColor(Qt::white)
{
	mRedProperty = new IntProperty;
	mRedProperty->setMinValue(0);
	mRedProperty->setMaxValue(255);
	mRedProperty->setPropertyName(tr("Red"));
	addChild(mRedProperty);

	mGreenProperty = new IntProperty;
	mGreenProperty->setMinValue(0);
	mGreenProperty->setMaxValue(255);
	mGreenProperty->setPropertyName(tr("Green"));
	addChild(mGreenProperty);

	mBlueProperty = new IntProperty;
	mBlueProperty->setMinValue(0);
	mBlueProperty->setMaxValue(255);
	mBlueProperty->setPropertyName(tr("Blue"));
	addChild(mBlueProperty);

	connect(mRedProperty, SIGNAL(valueChanged()), this, SLOT(dataChanged()));
	connect(mGreenProperty, SIGNAL(valueChanged()), this, SLOT(dataChanged()));
	connect(mBlueProperty, SIGNAL(valueChanged()), this, SLOT(dataChanged()));
}

ColorProperty::~ColorProperty()
{
	delete mRedProperty;
	delete mGreenProperty;
	delete mBlueProperty;
}

QString ColorProperty::valueText() const
{
	return mColor.name();
}

void ColorProperty::undo()
{
	setColor(mDefaultColor);
	setModified(false);
}

void ColorProperty::save()
{
	setDefaultColor(mColor);
	setModified(false);
}

QWidget* ColorProperty::editorWidget(QWidget* parent)
{
	if (!mWidget) {
		mWidget = new ColorButton(parent);

		mWidget->setEnabled(isEnabled());
		mWidget->setColor(mColor);
		mWidget->setFlat(true);

		connect(mWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(colorChanged(const QColor&)));
	}

	return mWidget;
}

void ColorProperty::colorChanged(const QColor& val)
{
	setColor(val);
}

void ColorProperty::setColor(const QColor& val)
{
	mColor = val;

	if (mColor != mDefaultColor && !isModified()) {
		setModified(true);
	} else if (mColor == mDefaultColor && isModified()) {
		setModified(false);
	}

	mRedProperty->blockSignals(true);
	mGreenProperty->blockSignals(true);
	mBlueProperty->blockSignals(true);

	mRedProperty->setValue(mColor.red());
	mGreenProperty->setValue(mColor.green());
	mBlueProperty->setValue(mColor.blue());

	mRedProperty->blockSignals(false);
	mGreenProperty->blockSignals(false);
	mBlueProperty->blockSignals(false);

	if (mWidget) {
		mWidget->blockSignals(true);
		mWidget->setColor(mColor);
		mWidget->blockSignals(false);
	}

	emit valueChanged();
}

QColor ColorProperty::color() const
{
	return mColor;
}

void ColorProperty::setDefaultColor(const QColor& val)
{
	mDefaultColor = val;

	if (mColor != mDefaultColor && !isModified()) {
		setModified(true);
	} else if (mColor == mDefaultColor && isModified()) {
		setModified(false);
	}
}

QColor ColorProperty::defaultColor() const
{
	return mDefaultColor;
}

void ColorProperty::dataChanged()
{
	setColor(QColor(mRedProperty->value(), mGreenProperty->value(), mBlueProperty->value()));
}
}