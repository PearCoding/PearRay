#pragma once

#include "IProperty.h"

#include "PR_Config.h"

namespace PR {
namespace UI {
class DoubleProperty;
class PR_LIB_UI Vector3Property : public IProperty {
	Q_OBJECT
public:
	Vector3Property();
	virtual ~Vector3Property();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	void setValue(const Vector3f& c);
	Vector3f value() const;

	void setDefaultValue(const Vector3f& c);
	Vector3f defaultValue() const;

private slots:
	void dataChanged();

private:
	Vector3f mDefaultValue;
	Vector3f mValue;

	DoubleProperty* mXProperty;
	DoubleProperty* mYProperty;
	DoubleProperty* mZProperty;
};
} // namespace UI
} // namespace PR