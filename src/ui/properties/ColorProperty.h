#pragma once

#include "IProperty.h"

#include <QColor>

namespace PR {
namespace UI {
class IntProperty;
class PR_LIB_UI ColorProperty : public IProperty {
	Q_OBJECT
public:
	ColorProperty();
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
	void colorChanged(const QColor&);
	void dataChanged();

private:
	QColor mDefaultColor;
	QColor mColor;

	IntProperty* mRedProperty;
	IntProperty* mGreenProperty;
	IntProperty* mBlueProperty;
};
} // namespace UI
} // namespace PR