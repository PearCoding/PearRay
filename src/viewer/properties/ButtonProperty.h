#pragma once

#include "IProperty.h"

#include <QPushButton>

class IntProperty;
class ButtonProperty : public IProperty
{
	Q_OBJECT
public:
	ButtonProperty(const QString& name);
	virtual ~ButtonProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

private: 
	QPushButton* mWidget;
};
