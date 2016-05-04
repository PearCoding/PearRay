#pragma once

#include "IProperty.h"

#include <QColor>

class ColorButton;
class IntProperty;
class ColorProperty : public IProperty
{
	Q_OBJECT
public:
	ColorProperty(const QString& name, const QColor& color = Qt::black);
	virtual ~ColorProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	void setColor(const QColor& c);
	QColor color() const;

	void setDefaultColor(const QColor& c);
	QColor defaultColor() const;

private slots:
	void colorChanged(const QColor &);
	void dataChanged();

private: 
	ColorButton* mWidget;
	QColor mDefaultColor;
	QColor mColor;

	IntProperty* mRedProperty;
	IntProperty* mGreenProperty;
	IntProperty* mBlueProperty;
};
