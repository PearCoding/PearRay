#pragma once

#include <QWidget>

#include "PR_Config.h"

namespace PRUI {
namespace Ui {
	class ToneMapperEditorClass;
}
class ToneMapper;
class PR_LIB_UI ToneMapperEditor : public QWidget {
	Q_OBJECT

public:
	ToneMapperEditor(QWidget* parent = 0);
	~ToneMapperEditor();

	void setMinMax(float min, float max);
	void setToNormal();
	ToneMapper constructMapper() const;

signals:
	void changed();
	void formatChanged();

private slots:
	void valueChanged();
	void normalButtonPushed();
	void fitButtonPushed();

private:
	Ui::ToneMapperEditorClass* ui;
};
}