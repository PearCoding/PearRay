#pragma once

#include "IProperty.h"

class QSpinBox;
class IntProperty : public IProperty
{
	Q_OBJECT
public:
	IntProperty(const QString& name, int value, int min = 0, int max = 1000000, int stepsize = 1);
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
	QSpinBox* mSpinBox;
	int mValue;
	int mOldValue;

	int mMaxValue;
	int mMinValue;
	int mStepSize;
};
