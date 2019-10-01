#pragma once

#include "ui_EXRWindow.h"
#include <QWidget>

#include <memory>

class EXRFile;
class EXRWindow : public QWidget {
	Q_OBJECT

public:
	EXRWindow(QWidget* parent = 0);
	~EXRWindow();

	void openFile(const QString& str);

private slots:
	void layerChanged();

private:
	void updateImage(int layerID);

	Ui::EXRWindowClass ui;

	std::unique_ptr<EXRFile> mFile;
};