#pragma once

#include "IProperty.h"

class QDoubleSpinBox;
class DoubleProperty : public IProperty
{
	Q_OBJECT
public:
	DoubleProperty(const QString& name, double value, double min = 0, double max = 1, double stepsize = 0.1, int decimals = 4);
	virtual ~DoubleProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	double maxValue() const;
	double minValue() const;
	double stepSize() const;
	int decimals() const;

	void setMaxValue(double i);
	void setMinValue(double i);
	void setStepSize(double i);
	void setDecimals(int i);

	void setValue(double val);
	double value() const;

	void setDefaultValue(double val);
	double defaultValue() const;

private slots:
	void spinBoxChanged(double val);

private: 
	QDoubleSpinBox* mSpinBox;
	double mOldValue;
	double mValue;

	double mMaxValue;
	double mMinValue;
	double mStepSize;
	int mDecimals;
};
