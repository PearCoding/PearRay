#pragma once

#include "IProperty.h"

namespace PR {
namespace UI {
class PR_LIB_UI IntProperty : public IProperty {
	Q_OBJECT
public:
	IntProperty();
	virtual ~IntProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	int maxValue() const;
	int minValue() const;
	int stepSize() const;

	void setMaxValue(int i);
	void setMinValue(int i);
	void setStepSize(int i);

	void setValue(int val);
	int value() const;

	void setDefaultValue(int val);
	int defaultValue() const;

private slots:
	void spinBoxChanged(int val);

private:
	int mOldValue;
	int mValue;

	int mMaxValue;
	int mMinValue;
	int mStepSize;
};
} // namespace UI
} // namespace PR