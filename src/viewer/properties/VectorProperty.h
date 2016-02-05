#pragma once

#include "IProperty.h"

class QDoubleSpinBox;
class VectorProperty : public IProperty
{
	Q_OBJECT
public:
	static constexpr int MAX_DIM = 4;

	VectorProperty(int dimension = 3);
	virtual ~VectorProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	double maxValue(int dim) const;
	double minValue(int dim) const;
	double stepSize(int dim) const;
	int decimals(int dim) const;

	void setMaxValue(double i, int dim);
	void setMinValue(double i, int dim);
	void setStepSize(double i, int dim);
	void setDecimals(int i, int dim);

	void setValue(double val, int dim);
	double value(int dim) const;

	void setDefaultValue(double val, int dim);
	double defaultValue(int dim) const;

private slots:
	void spinBoxChanged1(double val);
	void spinBoxChanged2(double val);
	void spinBoxChanged3(double val);
	void spinBoxChanged4(double val);

private: 
	const int mDimension;

	QDoubleSpinBox* mSpinBox[4];
	double mOldValue[4];
	double mValue[4];

	double mMaxValue[4];
	double mMinValue[4];
	double mStepSize[4];
	int mDecimals[4];
};
