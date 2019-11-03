#pragma once

#include "IProperty.h"

class GroupProperty : public IProperty {
	Q_OBJECT
public:
	GroupProperty();
	virtual ~GroupProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);
};
