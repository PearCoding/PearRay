#pragma once

#include "ui_SpecWindow.h"
#include <QWidget>

#include <memory>

class SpecWindow : public QWidget {
	Q_OBJECT

public:
	SpecWindow(QWidget* parent = 0);
	~SpecWindow();

	void openFile(const QString& str);

private slots:
	void exportImage();
	void updateMapper();

private:
	Ui::SpecWindowClass ui;
};