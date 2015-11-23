#pragma once

#include "IProperty.h"

class QCheckBox;
class BoolProperty : public IProperty
{
	Q_OBJECT
public:
	BoolProperty();
	virtual ~BoolProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);


	void setValue(bool val);
	bool value() const;

	void setDefaultValue(bool val);
	bool defaultValue() const;

private slots:
	void checkBoxChanged(int val);

private: 
	QCheckBox* mCheckBox;
	bool mOldValue;
	bool mValue;
};