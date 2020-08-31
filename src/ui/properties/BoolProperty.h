#pragma once

#include "IProperty.h"

namespace PRUI {
class PR_LIB_UI BoolProperty : public IProperty {
	Q_OBJECT
public:
	BoolProperty();
	virtual ~BoolProperty();

	QString valueText() const override;
	void undo() override;
	void save() override;
	QWidget* editorWidget(QWidget* parent) override;

	void setValue(bool val);
	bool value() const;

	void setDefaultValue(bool val);
	bool defaultValue() const;

private slots:
	void stateChanged(int state);

private:
	bool mOldValue;
	bool mValue;
};
}