#pragma once

#include "IProperty.h"

namespace PR {
namespace UI {
class PR_LIB_UI GroupProperty : public IProperty {
	Q_OBJECT
public:
	GroupProperty();
	virtual ~GroupProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);
};
} // namespace UI
} // namespace PR