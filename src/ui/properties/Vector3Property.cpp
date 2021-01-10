#include "Vector3Property.h"
#include "DoubleProperty.h"

#include "widgets/ColorButton.h"

namespace PR {
namespace UI {
Vector3Property::Vector3Property()
	: IProperty()
	, mDefaultValue(Vector3f::Zero())
	, mValue(Vector3f::Zero())
{
	mXProperty = new DoubleProperty;
	mXProperty->setPropertyName(tr("X"));
	addChild(mXProperty);

	mYProperty = new DoubleProperty;
	mYProperty->setPropertyName(tr("Y"));
	addChild(mYProperty);

	mZProperty = new DoubleProperty;
	mZProperty->setPropertyName(tr("Z"));
	addChild(mZProperty);

	connect(mXProperty, &DoubleProperty::valueChanged, this, &Vector3Property::dataChanged);
	connect(mYProperty, &DoubleProperty::valueChanged, this, &Vector3Property::dataChanged);
	connect(mZProperty, &DoubleProperty::valueChanged, this, &Vector3Property::dataChanged);
}

Vector3Property::~Vector3Property()
{
	delete mXProperty;
	delete mYProperty;
	delete mZProperty;
}

QString Vector3Property::valueText() const
{
	return QString("[%1, %2, %3]").arg(mValue(0)).arg(mValue(1)).arg(mValue(2));
}

void Vector3Property::undo()
{
	setValue(mDefaultValue);
	setModified(false);
}

void Vector3Property::save()
{
	setDefaultValue(mValue);
	setModified(false);
}

QWidget* Vector3Property::editorWidget(QWidget* parent)
{
	Q_UNUSED(parent);
	return nullptr;
}

void Vector3Property::setValue(const Vector3f& val)
{
	mValue = val;

	if (mValue != mDefaultValue && !isModified())
		setModified(true);
	else if (mValue == mDefaultValue && isModified())
		setModified(false);

	mXProperty->blockSignals(true);
	mYProperty->blockSignals(true);
	mZProperty->blockSignals(true);

	mXProperty->setValue(mValue(0));
	mYProperty->setValue(mValue(1));
	mZProperty->setValue(mValue(2));

	mXProperty->blockSignals(false);
	mYProperty->blockSignals(false);
	mZProperty->blockSignals(false);

	emit valueChanged();
}

Vector3f Vector3Property::value() const
{
	return mValue;
}

void Vector3Property::setDefaultValue(const Vector3f& val)
{
	mDefaultValue = val;

	if (mValue != mDefaultValue && !isModified())
		setModified(true);
	else if (mValue == mDefaultValue && isModified())
		setModified(false);
}

Vector3f Vector3Property::defaultValue() const
{
	return mDefaultValue;
}

void Vector3Property::dataChanged()
{
	setValue(Vector3f(mXProperty->value(), mYProperty->value(), mZProperty->value()));
}
} // namespace UI
} // namespace PR