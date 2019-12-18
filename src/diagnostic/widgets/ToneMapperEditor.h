#pragma once

#include "ui_ToneMapperEditor.h"
#include <QWidget>

#include <memory>

class ToneMapper;
class ToneMapperEditor : public QWidget {
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
	Ui::ToneMapperEditorClass ui;
};